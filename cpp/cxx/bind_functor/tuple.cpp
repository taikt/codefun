#include <tuple>
#include <cstddef>
#include <string>
#include <utility>
#include <iostream>
using namespace std;

template < ::std::size_t... Indices>
struct indices {};

template < ::std::size_t N, ::std::size_t... Is>
struct build_indices : build_indices<N-1, N-1, Is...>
{};

template < ::std::size_t... Is>
struct build_indices<0, Is...> : indices<Is...>
{};

template <typename FuncT, typename ArgTuple, ::std::size_t... Indices>
auto call(const FuncT &f, ArgTuple &&args, const indices<Indices...> &)
   -> decltype(f(::std::get<Indices>(::std::forward<ArgTuple>(args))...))
{
   return ::std::move(f(::std::get<Indices>(::std::forward<ArgTuple>(args))...));
}

template <typename FuncT, typename ArgTuple>
auto call(const FuncT &f, ArgTuple &&args)
     -> decltype(call(f, args,
                      build_indices< ::std::tuple_size<ArgTuple>::value>{}))
{
    const build_indices< ::std::tuple_size<ArgTuple>::value> indices;

    return ::std::move(call(f, ::std::move(args), indices));
}



int myfunc(::std::string name, int foo)
{
   cout<<"name is:"<<name<<", value:"<<foo<<"\n";
}

int main() {
	std::tuple<std::string, int> t = std::make_tuple("taikt",1);
	call(myfunc, std::move(t));

	
	
}





