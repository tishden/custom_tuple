#ifndef CUSTOM_TUPLE_CUSTOMTUPLE_H
#define CUSTOM_TUPLE_CUSTOMTUPLE_H

template<typename T>
concept is_custom_tuple = requires {
    T::IsCustomTuple;
};

template<typename T>
concept is_empty_custom_tuple = T::IsEmptyCustomTuple;

template<typename T>
concept is_not_empty_custom_tuple = is_custom_tuple<T> && !is_empty_custom_tuple<T>;

template<typename T, typename CT>
concept has = CT::template has<T>();

template<typename T, typename... Items>
concept is_duplicate = (... || (std::is_same_v<T, Items> || has<T, Items>));

template<typename ... Args>
class CustomTuple;

template<>
class CustomTuple<> {
public:
    static constexpr bool IsCustomTuple = true;
    static constexpr bool IsEmptyCustomTuple = true;

    template<typename Handler>
    void apply(const Handler &handler) {
    }

    template<typename Comparator, typename Handler>
    void applyIf(const Handler &handler) {
    }

    template<typename T>
    static constexpr bool has() {
        return false;
    }
};

template<typename Head, typename ... Tail>
class CustomTuple<Head, Tail...> : CustomTuple<Tail...> {
public:
    static constexpr bool IsCustomTuple = true;
    static constexpr bool IsEmptyCustomTuple = false;

    template<template<typename> typename Mapper>
    using Map = CustomTuple<typename Mapper<Head>::type, typename Mapper<Tail>::type...>;

    using TailType = CustomTuple<Tail...>;
    using HeadType = Head;
    HeadType head;

    template<typename T>
    void set(const T &value) {
        TailType::template set<T>(value);
    }

    template<>
    void set<HeadType>(const HeadType &value) {
        head = value;
    }

    template<typename T>
    T &get() {
        return TailType::template get<T>();
    }

    template<>
    HeadType &get<HeadType>() {
        return head;
    }

    template<typename T>
    static constexpr bool has() {
        return TailType::template has<T>();
    }

    template<>
    static constexpr bool has<HeadType>() {
        return true;
    }

    template<typename Handler>
    void apply(const Handler &handler) {
        handler(head);
        TailType::template apply<Handler>(handler);
    }

    template<typename Comparator, typename Handler>
    void applyIf(const Handler &handler) {
        if constexpr (Comparator::template check<HeadType>()) {
            handler(head);
        }
        TailType::template applyIf<Comparator, Handler>(handler);
    }
};

// make flat
template<is_not_empty_custom_tuple Head, typename ... Tail>
class CustomTuple<Head, Tail...> :
        public CustomTuple<typename Head::HeadType, typename Head::TailType, Tail...> {
};

// remove empty custom tuples
template<is_empty_custom_tuple Head, typename ... Tail>
class CustomTuple<Head, Tail...> :
        public CustomTuple<Tail...> {
};

// remove duplicates
template<typename Head, typename ... Tail>
requires is_duplicate<Head, Tail...> && (!is_custom_tuple<Head>)
class CustomTuple<Head, Tail...> : public CustomTuple<Tail...> {
};

#endif //CUSTOM_TUPLE_CUSTOMTUPLE_H
