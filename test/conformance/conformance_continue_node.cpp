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

//! \file conformance_continue_node.cpp
//! \brief Test for [flow_graph.continue_node] specification

void test_cont_body(){
    oneapi::tbb::flow::graph g;
    counting_functor<int> cf;
    cf.execute_count = 0;

    oneapi::tbb::flow::continue_node<int> node1(g, cf);

    const size_t n = 10;
    for(size_t i = 0; i < n; ++i) {
        CHECK_MESSAGE((node1.try_put(oneapi::tbb::flow::continue_msg()) == true),
                      "continue_node::try_put() should never reject a message.");
    }
    g.wait_for_all();

    CHECK_MESSAGE( (cf.execute_count == n), "Body of the first node needs to be executed N times");
}

template<typename O>
void test_inheritance(){
    using namespace oneapi::tbb::flow;

    CHECK_MESSAGE( (std::is_base_of<graph_node, continue_node<O>>::value), "continue_node should be derived from graph_node");
    CHECK_MESSAGE( (std::is_base_of<receiver<continue_msg>, continue_node<O>>::value), "continue_node should be derived from receiver<Input>");
    CHECK_MESSAGE( (std::is_base_of<sender<O>, continue_node<O>>::value), "continue_node should be derived from sender<Output>");
}

#if __TBB_CPP17_DEDUCTION_GUIDES_PRESENT

template <typename ExpectedType, typename Body>
void test_deduction_guides_common(Body body) {
    using namespace tbb::flow;
    graph g;

    continue_node c1(g, body);
    static_assert(std::is_same_v<decltype(c1), continue_node<ExpectedType>>);

    continue_node c2(g, body, lightweight());
    static_assert(std::is_same_v<decltype(c2), continue_node<ExpectedType, lightweight>>);

    continue_node c3(g, 5, body);
    static_assert(std::is_same_v<decltype(c3), continue_node<ExpectedType>>);

    continue_node c4(g, 5, body, lightweight());
    static_assert(std::is_same_v<decltype(c4), continue_node<ExpectedType, lightweight>>);

    continue_node c5(g, body, node_priority_t(5));
    static_assert(std::is_same_v<decltype(c5), continue_node<ExpectedType>>);

    continue_node c6(g, body, lightweight(), node_priority_t(5));
    static_assert(std::is_same_v<decltype(c6), continue_node<ExpectedType, lightweight>>);

    continue_node c7(g, 5, body, node_priority_t(5));
    static_assert(std::is_same_v<decltype(c7), continue_node<ExpectedType>>);

    continue_node c8(g, 5, body, lightweight(), node_priority_t(5));
    static_assert(std::is_same_v<decltype(c8), continue_node<ExpectedType, lightweight>>);

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    broadcast_node<continue_msg> b(g);

    continue_node c9(follows(b), body);
    static_assert(std::is_same_v<decltype(c9), continue_node<ExpectedType>>);

    continue_node c10(follows(b), body, lightweight());
    static_assert(std::is_same_v<decltype(c10), continue_node<ExpectedType, lightweight>>);

    continue_node c11(follows(b), 5, body);
    static_assert(std::is_same_v<decltype(c11), continue_node<ExpectedType>>);

    continue_node c12(follows(b), 5, body, lightweight());
    static_assert(std::is_same_v<decltype(c12), continue_node<ExpectedType, lightweight>>);

    continue_node c13(follows(b), body, node_priority_t(5));
    static_assert(std::is_same_v<decltype(c13), continue_node<ExpectedType>>);

    continue_node c14(follows(b), body, lightweight(), node_priority_t(5));
    static_assert(std::is_same_v<decltype(c14), continue_node<ExpectedType, lightweight>>);

    continue_node c15(follows(b), 5, body, node_priority_t(5));
    static_assert(std::is_same_v<decltype(c15), continue_node<ExpectedType>>);

    continue_node c16(follows(b), 5, body, lightweight(), node_priority_t(5));
    static_assert(std::is_same_v<decltype(c16), continue_node<ExpectedType, lightweight>>);
#endif // __TBB_PREVIEW_FLOW_GRAPH_NODE_SET

    continue_node c17(c1);
    static_assert(std::is_same_v<decltype(c17), continue_node<ExpectedType>>);
}

int continue_body_f(const tbb::flow::continue_msg&) { return 1; }
void continue_void_body_f(const tbb::flow::continue_msg&) {}

void test_deduction_guides() {
    using tbb::flow::continue_msg;
    test_deduction_guides_common<int>([](const continue_msg&)->int { return 1; } );
    test_deduction_guides_common<continue_msg>([](const continue_msg&) {});

    test_deduction_guides_common<int>([](const continue_msg&) mutable ->int { return 1; });
    test_deduction_guides_common<continue_msg>([](const continue_msg&) mutable {});

    test_deduction_guides_common<int>(continue_body_f);
    test_deduction_guides_common<continue_msg>(continue_void_body_f);
}

#endif // __TBB_CPP17_DEDUCTION_GUIDES_PRESENT

void test_forwarding(){
    oneapi::tbb::flow::graph g;
    constexpr int expected = 5;
    counting_functor<int> fun(expected);
    fun.execute_count = 0;

    oneapi::tbb::flow::continue_node<int> node1(g, fun);
    test_push_receiver<int> node2(g);
    test_push_receiver<int> node3(g);

    oneapi::tbb::flow::make_edge(node1, node2);
    oneapi::tbb::flow::make_edge(node1, node3);

    node1.try_put(oneapi::tbb::flow::continue_msg());
    g.wait_for_all();

    auto values2 = get_values(node2);
    auto values3 = get_values(node3);

    CHECK_MESSAGE( (values2.size() == 1), "Descendant of the node must receive one message.");
    CHECK_MESSAGE( (values3.size() == 1), "Descendant of the node must receive one message.");
    CHECK_MESSAGE( (values2[0] == expected), "Value passed is the actual one received.");
    CHECK_MESSAGE( (values2 == values3), "Value passed is the actual one received.");
}

void test_buffering(){
    oneapi::tbb::flow::graph g;
    dummy_functor<int> fun;

    oneapi::tbb::flow::continue_node<int> node(g, fun);
    oneapi::tbb::flow::limiter_node<int> rejecter(g, 0);

    oneapi::tbb::flow::make_edge(node, rejecter);
    node.try_put(oneapi::tbb::flow::continue_msg());

    int tmp = -1;
    CHECK_MESSAGE( (node.try_get(tmp) == false), "try_get after rejection should not succeed");
    CHECK_MESSAGE( (tmp == -1), "try_get after rejection should not alter passed value");
    g.wait_for_all();
}

void test_ctors(){
    using namespace oneapi::tbb::flow;
    graph g;

    counting_functor<int> fun;

    continue_node<int> proto1(g, fun);
    continue_node<int> proto2(g, fun, oneapi::tbb::flow::node_priority_t(1));
    continue_node<int> proto3(g, 2, fun);
    continue_node<int> proto4(g, 2, fun, oneapi::tbb::flow::node_priority_t(1));

    continue_node<int, lightweight> lw_node1(g, fun, lightweight());
    continue_node<int, lightweight> lw_node2(g, fun, lightweight(), oneapi::tbb::flow::node_priority_t(1));
    continue_node<int, lightweight> lw_node3(g, 2, fun, lightweight());
    continue_node<int, lightweight> lw_node4(g, 2, fun, lightweight(), oneapi::tbb::flow::node_priority_t(1));
}

void test_copy_ctor(){
    using namespace oneapi::tbb::flow;
    graph g;

    counting_functor<int> fun;

    continue_node<oneapi::tbb::flow::continue_msg> node0(g, fun);
    continue_node<oneapi::tbb::flow::continue_msg> node1(g, 2, fun);
    test_push_receiver<oneapi::tbb::flow::continue_msg> node2(g);
    test_push_receiver<oneapi::tbb::flow::continue_msg> node3(g);

    oneapi::tbb::flow::make_edge(node0, node1);
    oneapi::tbb::flow::make_edge(node1, node2);

    continue_node<oneapi::tbb::flow::continue_msg> node_copy(node1);

    oneapi::tbb::flow::make_edge(node_copy, node3);

    node_copy.try_put(oneapi::tbb::flow::continue_msg());
    g.wait_for_all();

    CHECK_MESSAGE( (get_values(node2).size() == 0 && get_values(node3).size() == 0), "Copied node doesn`t copy successor, but copy number of predecessors");

    node_copy.try_put(oneapi::tbb::flow::continue_msg());
    g.wait_for_all();

    CHECK_MESSAGE( (get_values(node2).size() == 0 && get_values(node3).size() == 1), "Copied node doesn`t copy successor, but copy number of predecessors");

    node1.try_put(oneapi::tbb::flow::continue_msg());
    node1.try_put(oneapi::tbb::flow::continue_msg());
    node0.try_put(oneapi::tbb::flow::continue_msg());
    g.wait_for_all();

    CHECK_MESSAGE( (get_values(node2).size() == 1 && get_values(node3).size() == 0), "Copied node doesn`t copy predecessor, but copy number of predecessors");
}

void test_copies(){
    using namespace oneapi::tbb::flow;

    CountingObject<int> b;

    graph g;
    continue_node<int> fn(g, b);

    CountingObject<int> b2 = copy_body<CountingObject<int>,
                                             continue_node<int>>(fn);

    CHECK_MESSAGE( (b.copy_count + 2 <= b2.copy_count), "copy_body and constructor should copy bodies");
    CHECK_MESSAGE( (b.is_copy != b2.is_copy), "copy_body and constructor should copy bodies");
}

void test_output_class(){
    using namespace oneapi::tbb::flow;

    passthru_body<CountingObject<int>> fun;

    graph g;
    continue_node<CountingObject<int>> node1(g, fun);
    test_push_receiver<CountingObject<int>> node2(g);
    make_edge(node1, node2);
    
    node1.try_put(oneapi::tbb::flow::continue_msg());
    g.wait_for_all();
    CountingObject<int> b;
    node2.try_get(b);
    DOCTEST_WARN_MESSAGE( (b.is_copy), "The type Output must meet the CopyConstructible requirements");
}


void test_priority(){
    size_t concurrency_limit = 1;
    oneapi::tbb::global_control control(oneapi::tbb::global_control::max_allowed_parallelism, concurrency_limit);

    oneapi::tbb::flow::graph g;

    oneapi::tbb::flow::continue_node<oneapi::tbb::flow::continue_msg> source(g,
                                                             [](oneapi::tbb::flow::continue_msg){ return oneapi::tbb::flow::continue_msg();});

    first_functor<int>::first_id = -1;
    first_functor<int> low_functor(1);
    first_functor<int> high_functor(2);

    oneapi::tbb::flow::continue_node<int, int> high(g, high_functor, oneapi::tbb::flow::node_priority_t(1));
    oneapi::tbb::flow::continue_node<int, int> low(g, low_functor);

    make_edge(source, low);
    make_edge(source, high);

    source.try_put(oneapi::tbb::flow::continue_msg());

    g.wait_for_all();
    
    CHECK_MESSAGE( (first_functor<int>::first_id == 2), "High priority node should execute first");
}

void test_number_of_predecessors(){
    oneapi::tbb::flow::graph g;

    counting_functor<int> fun;
    fun.execute_count = 0;

    oneapi::tbb::flow::continue_node<oneapi::tbb::flow::continue_msg> node1(g, fun);
    oneapi::tbb::flow::continue_node<oneapi::tbb::flow::continue_msg> node2(g, 1, fun);
    oneapi::tbb::flow::continue_node<oneapi::tbb::flow::continue_msg> node3(g, 1, fun);
    oneapi::tbb::flow::continue_node<int> node4(g, fun); 

    oneapi::tbb::flow::make_edge(node1, node2);
    oneapi::tbb::flow::make_edge(node2, node4);
    oneapi::tbb::flow::make_edge(node1, node3);
    oneapi::tbb::flow::make_edge(node1, node3);
    oneapi::tbb::flow::remove_edge(node1, node3);
    oneapi::tbb::flow::make_edge(node3, node4);
    node3.try_put(oneapi::tbb::flow::continue_msg());
    node2.try_put(oneapi::tbb::flow::continue_msg());
    node1.try_put(oneapi::tbb::flow::continue_msg());

    g.wait_for_all();
    CHECK_MESSAGE( (fun.execute_count == 4), "Node wait for their predecessors to complete before executing");
}

void test_try_put() {
    barrier_body body;
    oneapi::tbb::flow::graph g;

    oneapi::tbb::flow::continue_node<oneapi::tbb::flow::continue_msg> node1(g, body);
    node1.try_put(oneapi::tbb::flow::continue_msg());
    barrier_body::flag.store(true);
    g.wait_for_all();
}

//! Test node costructors
//! \brief \ref requirement
TEST_CASE("continue_node constructors"){
    test_ctors();
}

//! Test node costructors
//! \brief \ref requirement
TEST_CASE("continue_node copy constructor"){
    test_copy_ctor();
}

//! Test priorities work in single-threaded configuration
//! \brief \ref requirement
TEST_CASE("continue_node priority support"){
    test_priority();
}

//! Test body copying and copy_body logic
//! \brief \ref interface
TEST_CASE("continue_node and body copying"){
    test_copies();
}

//! Test continue_node buffering
//! \brief \ref requirement
TEST_CASE("continue_node buffering"){
    test_buffering();
}

//! Test function_node broadcasting
//! \brief \ref requirement
TEST_CASE("continue_node broadcast"){
    test_forwarding();
}

//! Test deduction guides
//! \brief \ref interface \ref requirement
TEST_CASE("Deduction guides"){
#if __TBB_CPP17_DEDUCTION_GUIDES_PRESENT
    test_deduction_guides();
#endif
}

//! Test inheritance relations
//! \brief \ref interface
TEST_CASE("continue_node superclasses"){
    test_inheritance<int>();
    test_inheritance<void*>();
}

//! Test body execution
//! \brief \ref interface \ref requirement
TEST_CASE("continue body") {
    test_cont_body();
}

//! Test body execution
//! \brief \ref interface \ref requirement
TEST_CASE("continue_node number_of_predecessors") {
    test_number_of_predecessors();
}

//! Test body execution
//! \brief \ref interface \ref requirement
TEST_CASE("continue_node Output class") {
    test_output_class();
}

//! Test body execution
//! \brief \ref interface \ref requirement
TEST_CASE("continue_node `try_put' statement not wait for the execution of the body to complete.") {
    test_try_put();
}
