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

#ifndef __TBB_test_conformance_conformance_flowgraph_H
#define __TBB_test_conformance_conformance_flowgraph_H

template <typename OutputType = int>
struct passthru_body {
    OutputType operator()( const OutputType& i ) {
        return i;
    }

    OutputType operator()( oneapi::tbb::flow::continue_msg ) {
       return OutputType();
    }

    void operator()( const int& argument, oneapi::tbb::flow::multifunction_node<int, std::tuple<int>>::output_ports_type &op ) {
       std::get<0>(op).try_put(argument);
    }

};

template<typename V>
using test_push_receiver = oneapi::tbb::flow::queue_node<V>;

template<typename V>
std::vector<V> get_values( test_push_receiver<V>& rr ){
    std::vector<V> messages;
    int val = 0;
    for(V tmp; rr.try_get(tmp); ++val){
        messages.push_back(tmp);
    }
    return messages;
}

template< typename OutputType >
struct first_functor {
    int my_id;
    static std::atomic<int> first_id;

    first_functor(int id) : my_id(id) {}

    OutputType operator()( OutputType argument ) {
        int old_value = first_id;
        while(first_id == -1 &&
              !first_id.compare_exchange_weak(old_value, my_id))
            ;

        return argument;
    }

    OutputType operator()( const oneapi::tbb::flow::continue_msg&  ) {
        return operator()(OutputType());
    }

    void operator()( const OutputType& argument, oneapi::tbb::flow::multifunction_node<int, std::tuple<int>>::output_ports_type &op ) {
        operator()(OutputType());
        std::get<0>(op).try_put(argument);
    }
};

template<typename OutputType>
std::atomic<int> first_functor<OutputType>::first_id;

template< typename OutputType>
struct counting_functor {
    OutputType my_value;

    static std::atomic<size_t> execute_count;

    counting_functor(OutputType value = OutputType()) : my_value(value) {

    }

    OutputType operator()( oneapi::tbb::flow::continue_msg ) {
       ++execute_count;
       return my_value;
    }

    OutputType operator()( OutputType argument ) {
       ++execute_count;
       return argument;
    }
};

template<typename OutputType>
std::atomic<size_t> counting_functor<OutputType>::execute_count;

template< typename OutputType>
struct dummy_functor {
    OutputType operator()( oneapi::tbb::flow::continue_msg ) {
       return OutputType();
    }
};

struct barrier_body{
    static std::atomic<bool> flag;

    void operator()(oneapi::tbb::flow::continue_msg){
        while(!flag.load()){ };
    }
};

std::atomic<bool> barrier_body::flag{false};

template<typename O>
struct CountingObject{
    size_t copy_count;
    size_t assign_count;
    size_t move_count;
    bool is_copy;

    CountingObject():
        copy_count(0), assign_count(0), move_count(0), is_copy(false) {}

    CountingObject(const CountingObject<O>& other):
        copy_count(other.copy_count + 1), is_copy(true) {}

    CountingObject& operator=(const CountingObject<O>& other) {
        assign_count = other.assign_count + 1;
        is_copy = true;
        return *this;
    }

    CountingObject(CountingObject<O>&& other):
         copy_count(other.copy_count), move_count(other.move_count + 1), is_copy(other.is_copy) {}

    CountingObject& operator=(CountingObject<O>&& other) {
        copy_count = other.copy_count;
        is_copy = other.is_copy;
        move_count = other.move_count + 1;
        return *this;
    }

    O operator()(oneapi::tbb::flow::continue_msg){
        return 1;
    }

    O operator()(O){
        return 1;
    }
};

#endif // __TBB_test_conformance_conformance_flowgraph_H
