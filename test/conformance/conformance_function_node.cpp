/*
    Copyright (c) 2020-2021 Intel Corporation

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#if __INTEL_COMPILER && _MSC_VER
#pragma warning(disable : 2586) // decorated name length exceeded, name was truncated
#endif

#include "common/test.h"

#include "common/utils.h"
#include "common/graph_utils.h"

#include "oneapi/tbb/flow_graph.h"
#include "oneapi/tbb/task_arena.h"
#include "oneapi/tbb/global_control.h"

#include "conformance_flowgraph.h"

//! \file conformance_function_node.cpp
//! \brief Test for [flow_graph.function_node] specification

/*
TODO: implement missing conformance tests for function_node:
  - [X] Constructor with explicitly passed Policy parameter: `template<typename Body> function_node(
    graph &g, size_t concurrency, Body body, Policy(), node_priority_t, priority = no_priority )'
  - [X] Explicit test for copy constructor of the node.
  - [X] Rename test_broadcast to test_forwarding and check that the value passed is the actual one
    received.
  - [ ] Concurrency testing of the node: make a loop over possible concurrency levels. It is
    important to test at least on five values: 1, oneapi::tbb::flow::serial, `max_allowed_parallelism'
    obtained from `oneapi::tbb::global_control', `oneapi::tbb::flow::unlimited', and, if `max allowed
    parallelism' is > 2, use something in the middle of the [1, max_allowed_parallelism]
    interval. Use `utils::ExactConcurrencyLevel' entity (extending it if necessary).
  - [?] make `test_rejecting' deterministic, i.e. avoid dependency on OS scheduling of the threads;
    add check that `try_put()' returns `false'
  - [*] The copy constructor and copy assignment are called for the node's input and output types.l
  - [X] The `copy_body' function copies altered body (e.g. after successful `try_put()' call).
  - [X] Extend CTAD test to check all node's constructors.
*/

std::atomic<size_t> my_concurrency;
std::atomic<size_t> my_max_concurrency;

template< typename OutputType >
struct concurrency_functor {
    OutputType operator()( int argument ) {
        ++my_concurrency;

        size_t old_value = my_max_concurrency;
        while(my_max_concurrency < my_concurrency &&
              !my_max_concurrency.compare_exchange_weak(old_value, my_concurrency))
            ;

        size_t ms = 1000;
        std::chrono::milliseconds sleep_time( ms );
        std::this_thread::sleep_for( sleep_time );

        --my_concurrency;
       return argument;
    }

};

void test_func_body(){
    oneapi::tbb::flow::graph g;
    counting_functor<int> fun;
    fun.execute_count = 0;

    oneapi::tbb::flow::function_node<int, int> node1(g, oneapi::tbb::flow::unlimited, fun);

    const size_t n = 10;
    for(size_t i = 0; i < n; ++i) {
        CHECK_MESSAGE((node1.try_put(1) == true), "try_put needs to return true");
    }
    g.wait_for_all();

    CHECK_MESSAGE( (fun.execute_count == n), "Body of the node needs to be executed N times");
}

void test_priority(){
    size_t concurrency_limit = 1;
    oneapi::tbb::global_control control(oneapi::tbb::global_control::max_allowed_parallelism, concurrency_limit);

    oneapi::tbb::flow::graph g;

    first_functor<int>::first_id.store(-1);
    first_functor<int> low_functor(1);
    first_functor<int> high_functor(2);

    oneapi::tbb::flow::continue_node<int> source(g, [&](oneapi::tbb::flow::continue_msg){return 1;} );

    oneapi::tbb::flow::function_node<int, int> high(g, oneapi::tbb::flow::unlimited, high_functor, oneapi::tbb::flow::node_priority_t(1));
    oneapi::tbb::flow::function_node<int, int> low(g, oneapi::tbb::flow::unlimited, low_functor);

    make_edge(source, low);
    make_edge(source, high);

    source.try_put(oneapi::tbb::flow::continue_msg());
    g.wait_for_all();

    CHECK_MESSAGE( (first_functor<int>::first_id == 2), "High priority node should execute first");
}

#if __TBB_CPP17_DEDUCTION_GUIDES_PRESENT

int function_body_f(const int&) { return 1; }

template <typename Body>
void test_deduction_guides_common(Body body) {
    using namespace tbb::flow;
    graph g;

    function_node f1(g, unlimited, body);
    static_assert(std::is_same_v<decltype(f1), function_node<int, int>>);

    function_node f2(g, unlimited, body, rejecting());
    static_assert(std::is_same_v<decltype(f2), function_node<int, int, rejecting>>);

    function_node f3(g, unlimited, body, node_priority_t(5));
    static_assert(std::is_same_v<decltype(f3), function_node<int, int>>);

    function_node f4(g, unlimited, body, rejecting(), node_priority_t(5));
    static_assert(std::is_same_v<decltype(f4), function_node<int, int, rejecting>>);

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    function_node f5(follows(f2), unlimited, body);
    static_assert(std::is_same_v<decltype(f5), function_node<int, int>>);

    function_node f6(follows(f5), unlimited, body, rejecting());
    static_assert(std::is_same_v<decltype(f6), function_node<int, int, rejecting>>);

    function_node f7(follows(f6), unlimited, body, node_priority_t(5));
    static_assert(std::is_same_v<decltype(f7), function_node<int, int>>);

    function_node f8(follows(f7), unlimited, body, rejecting(), node_priority_t(5));
    static_assert(std::is_same_v<decltype(f8), function_node<int, int, rejecting>>);
#endif // __TBB_PREVIEW_FLOW_GRAPH_NODE_SET

    function_node f9(f1);
    static_assert(std::is_same_v<decltype(f9), function_node<int, int>>);
}

void test_deduction_guides() {
    test_deduction_guides_common([](const int&)->int { return 1; });
    test_deduction_guides_common([](const int&) mutable ->int { return 1; });
    test_deduction_guides_common(function_body_f);
}

#endif

void test_forvarding(){
    oneapi::tbb::flow::graph g;
    const int expected = 5;
    counting_functor<int> fun;
    fun.execute_count = 0;

    oneapi::tbb::flow::function_node<int, int> node1(g, oneapi::tbb::flow::unlimited, fun);
    test_push_receiver<int> node2(g);
    test_push_receiver<int> node3(g);

    oneapi::tbb::flow::make_edge(node1, node2);
    oneapi::tbb::flow::make_edge(node1, node3);

    node1.try_put(expected);
    g.wait_for_all();

    auto values2 = get_values(node2);
    auto values3 = get_values(node3);

    CHECK_MESSAGE( (values2.size() == 1), "Descendant of the node must receive one message.");
    CHECK_MESSAGE( (values3.size() == 1), "Descendant of the node must receive one message.");
    CHECK_MESSAGE( (values2[0] == expected), "Value passed is the actual one received.");
    CHECK_MESSAGE( (values2 == values3), "Value passed is the actual one received.");
}

template<typename Policy>
void test_buffering(){
    oneapi::tbb::flow::graph g;
    counting_functor<int> fun;

    oneapi::tbb::flow::function_node<int, int, Policy> node(g, oneapi::tbb::flow::unlimited, fun);
    oneapi::tbb::flow::limiter_node<int> rejecter(g, 0);

    oneapi::tbb::flow::make_edge(node, rejecter);
    node.try_put(1);

    int tmp = -1;
    CHECK_MESSAGE( (node.try_get(tmp) == false), "try_get after rejection should not succeed");
    CHECK_MESSAGE( (tmp == -1), "try_get after rejection should not alter passed value");
    g.wait_for_all();
}

void test_node_concurrency(){

    /*Concurrency testing of the node: make a loop over possible concurrency levels. It is
    important to test at least on five values: 1, oneapi::tbb::flow::serial, `max_allowed_parallelism'
    obtained from `oneapi::tbb::global_control', `oneapi::tbb::flow::unlimited', and, if `max allowed
    parallelism' is > 2, use something in the middle of the [1, max_allowed_parallelism]
    interval. Use `utils::ExactConcurrencyLevel' entity (extending it if necessary).*/
    my_concurrency = 0;
    my_max_concurrency = 0;

    oneapi::tbb::flow::graph g;
    concurrency_functor<int> counter;
    oneapi::tbb::flow::function_node <int, int> fnode(g, oneapi::tbb::flow::serial, counter);

    test_push_receiver<int> sink(g);

    make_edge(fnode, sink);

    for(int i = 0; i < 10; ++i){
        fnode.try_put(i);
    }

    g.wait_for_all();

    CHECK_MESSAGE( ( my_max_concurrency.load() == 1), "Measured parallelism is not expected");
}

template<typename I, typename O>
void test_inheritance(){
    using namespace oneapi::tbb::flow;

    CHECK_MESSAGE( (std::is_base_of<graph_node, function_node<I, O>>::value), "function_node should be derived from graph_node");
    CHECK_MESSAGE( (std::is_base_of<receiver<I>, function_node<I, O>>::value), "function_node should be derived from receiver<Input>");
    CHECK_MESSAGE( (std::is_base_of<sender<O>, function_node<I, O>>::value), "function_node should be derived from sender<Output>");
}

void test_ctors(){
    using namespace oneapi::tbb::flow;
    graph g;

    counting_functor<int> fun;

    function_node<int, int> fn1(g, unlimited, fun);
    function_node<int, int> fn2(g, unlimited, fun, oneapi::tbb::flow::node_priority_t(1));

    function_node<int, int, lightweight> lw_node1(g, serial, fun, lightweight());
    function_node<int, int, lightweight> lw_node2(g, serial, fun, lightweight(), oneapi::tbb::flow::node_priority_t(1));
}

void test_copy_ctor(){
    using namespace oneapi::tbb::flow;
    graph g;

    counting_functor<int> fun;

    function_node<int, int> node0(g, unlimited, fun);
    function_node<int, oneapi::tbb::flow::continue_msg> node1(g, unlimited, fun);
    test_push_receiver<oneapi::tbb::flow::continue_msg> node2(g);
    test_push_receiver<oneapi::tbb::flow::continue_msg> node3(g);

    oneapi::tbb::flow::make_edge(node0, node1);
    oneapi::tbb::flow::make_edge(node1, node2);

    function_node<int, oneapi::tbb::flow::continue_msg> node_copy(node1);

    oneapi::tbb::flow::make_edge(node_copy, node3);

    node_copy.try_put(1);
    g.wait_for_all();

    CHECK_MESSAGE( (get_values(node2).size() == 0 && get_values(node3).size() == 1), "Copied node doesn`t copy successor, but copy number of predecessors");

    node0.try_put(1);
    g.wait_for_all();

    CHECK_MESSAGE( (get_values(node2).size() == 1 && get_values(node3).size() == 0), "Copied node doesn`t copy predecessor, but copy number of predecessors");
    /*check copy body*/
}

void test_copies(){
    using namespace oneapi::tbb::flow;

    CountingObject<int> b;

    graph g;
    function_node<int, int> fn(g, unlimited, b);

    CountingObject<int> b2 = copy_body<CountingObject<int>, function_node<int, int>>(fn);

    CHECK_MESSAGE( (b.copy_count + 2 <= b2.copy_count), "copy_body and constructor should copy bodies");
    CHECK_MESSAGE( (b.is_copy != b2.is_copy), "copy_body and constructor should copy bodies");
}

void test_output_input_class(){
    using namespace oneapi::tbb::flow;

    passthru_body<CountingObject<int>> fun;

    graph g;
    function_node<oneapi::tbb::flow::continue_msg, CountingObject<int>> node1(g, unlimited, fun);
    function_node<CountingObject<int>, CountingObject<int>> node2(g, unlimited, fun);
    test_push_receiver<CountingObject<int>> node3(g);
    make_edge(node1, node2);
    make_edge(node2, node3);
    /*Check Input class*/
    node1.try_put(oneapi::tbb::flow::continue_msg());
    g.wait_for_all();
    CountingObject<int> b;
    node3.try_get(b);
    DOCTEST_WARN_MESSAGE( (b.copy_count == 1), "The type Output must meet the CopyConstructible requirements");
    DOCTEST_WARN_MESSAGE( (b.assign_count == 1), "The type Output must meet the CopyConstructible requirements");
}

void test_rejecting(){
    oneapi::tbb::flow::graph g;
    oneapi::tbb::flow::function_node <int, int, oneapi::tbb::flow::rejecting> fnode(g, oneapi::tbb::flow::serial,
                                                                    [&](int v){
                                                                        size_t ms = 50;
                                                                        std::chrono::milliseconds sleep_time( ms );
                                                                        std::this_thread::sleep_for( sleep_time );
                                                                        return v;
                                                                    });

    test_push_receiver<int> sink(g);

    make_edge(fnode, sink);

    bool try_put_state;

    for(int i = 0; i < 10; ++i){
        try_put_state = fnode.try_put(i);
    }

    g.wait_for_all();
    CHECK_MESSAGE( (get_values(sink).size() == 1), "Messages should be rejected while the first is being processed");
    CHECK_MESSAGE( (!try_put_state), "`try_put()' should returns `false' after rejecting");
}

//! Test function_node costructors
//! \brief \ref requirement
TEST_CASE("function_node constructors"){
    test_ctors();
}

//! Test function_node costructors
//! \brief \ref requirement
TEST_CASE("function_node copy constructor"){
    test_copy_ctor();
}

//! Test function_node with rejecting policy
//! \brief \ref interface
TEST_CASE("function_node with rejecting policy"){
    test_rejecting();
}

//! Test body copying and copy_body logic
//! \brief \ref interface
TEST_CASE("function_node and body copying"){
    test_copies();
}

//! Test inheritance relations
//! \brief \ref interface
TEST_CASE("function_node superclasses"){
    test_inheritance<int, int>();
    test_inheritance<void*, float>();
}

//! Test function_node buffering
//! \brief \ref requirement
TEST_CASE("function_node buffering"){
    test_buffering<oneapi::tbb::flow::rejecting>();
    test_buffering<oneapi::tbb::flow::queueing>();
}

//! Test function_node broadcasting
//! \brief \ref requirement
TEST_CASE("function_node broadcast"){
    test_forvarding();
}

//! Test deduction guides
//! \brief \ref interface \ref requirement
TEST_CASE("Deduction guides"){
#if __TBB_CPP17_DEDUCTION_GUIDES_PRESENT
    test_deduction_guides();
#endif
}

//! Test priorities work in single-threaded configuration
//! \brief \ref requirement
TEST_CASE("function_node priority support"){
    test_priority();
}

//! Test that measured concurrency respects set limits
//! \brief \ref requirement
TEST_CASE("concurrency follows set limits"){
    test_node_concurrency();
}

//! Test calling function body
//! \brief \ref interface \ref requirement
TEST_CASE("Test function_node body") {
    test_func_body();
}
