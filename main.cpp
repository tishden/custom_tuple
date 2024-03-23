#include <iostream>
#include <cstdint>
#include "core/CustomTuple.h"
#include "vector"
#include "algorithm"
#include "memory"

enum Instrument {
    USDRUB = 0,
    USDRUB_F = 1,
    AFLT = 2
};

template<const Instrument instrument>
constexpr std::string_view instrument_to_string() {
    switch (instrument) {
        case USDRUB:
            return "USDRUB";
        case USDRUB_F:
            return "USDRUB_F";
        case AFLT:
            return "AFLT";
    }
    return "UNKNOWN";
};
enum class Venue {
    MOEX_FX,
    MOEX_EQUITY,
    MOEX_FUTURES
};

template<Instrument INSTRUMENT, Venue VENUE>
class InstrumentDefinition {
public:
    constexpr static Instrument InstrumentType = INSTRUMENT;
    constexpr static Venue VenueType = VENUE;
};

struct MarketDataPackage {
public:
    explicit MarketDataPackage(Instrument instrument) : instrument(instrument) {}

    Instrument instrument;
    std::uint8_t data{};
};

class ArbitrageStrategy {
public:
    using Instruments = CustomTuple<
            InstrumentDefinition<USDRUB, Venue::MOEX_FX>,
            InstrumentDefinition<USDRUB_F, Venue::MOEX_FUTURES>>;

    template<typename INSTRUMENT_DEFINITION>
    void makeMoney(const MarketDataPackage &data) {
        std::cout << "ArbitrageStrategy: making money for "
                  << instrument_to_string<INSTRUMENT_DEFINITION::InstrumentType>() << std::endl;
    }
};

class MarketMakingStrategy {
public:
    using Instruments = CustomTuple<
            InstrumentDefinition<AFLT, Venue::MOEX_EQUITY>>;

    template<typename INSTRUMENT_DEFINITION>
    void makeMoney(const MarketDataPackage &data) {
        std::cout << "MarketMakingStrategy: making money for "
                  << instrument_to_string<INSTRUMENT_DEFINITION::InstrumentType>() << std::endl;
    }
};

class MarketMakingStrategyFx {
public:
    using Instruments = CustomTuple<
            InstrumentDefinition<USDRUB, Venue::MOEX_FX>>;

    template<typename INSTRUMENT_DEFINITION>
    void makeMoney(const MarketDataPackage &data) {
        std::cout << "MarketMakingStrategyFx: making money for "
                  << instrument_to_string<INSTRUMENT_DEFINITION::InstrumentType>() << std::endl;
    }
};

class AbstractStrategy {
public:
    AbstractStrategy(std::string_view name, const Instrument instrument, const Venue venue) :
            name(name), instrument(instrument), venue(venue) {}

    void makeMoney(const MarketDataPackage &data) {
        std::cout << name << " making money for ";
        switch (instrument) {
            case USDRUB: {
                std::cout << instrument_to_string<USDRUB>() << std::endl;
                break;
            }
            case USDRUB_F: {
                std::cout << instrument_to_string<USDRUB_F>() << std::endl;
                break;
            }
            case AFLT: {
                std::cout << instrument_to_string<AFLT>() << std::endl;
                break;
            }
        }
    }

private:
    const std::string name;
    const Instrument instrument;
    const Venue venue;
};

class VirtualStrategy {
public:
    VirtualStrategy(std::string_view name, const Instrument instrument, const Venue venue) :
            name(name), instrument(instrument), venue(venue) {}

    virtual ~VirtualStrategy() {
    }

    virtual void makeMoney(const MarketDataPackage &data) = 0;

protected:
    const std::string name;
    const Instrument instrument;
    const Venue venue;
};

class VirtualArbitrageStrategy: public virtual VirtualStrategy {
public:
    VirtualArbitrageStrategy()
            : VirtualStrategy("VirtualArbitrageStrategy", USDRUB, Venue::MOEX_FX) {}

    virtual void makeMoney(const MarketDataPackage &data) {
        std::cout << VirtualStrategy::name << " making money for " << instrument_to_string<USDRUB>() << std::endl;
    }
};

class VirtualMarketMakingStrategy: public virtual VirtualStrategy {
public:
    VirtualMarketMakingStrategy()
            : VirtualStrategy("VirtualMarketMakingStrategy", USDRUB, Venue::MOEX_FX) {}

    virtual void makeMoney(const MarketDataPackage &data) {
        std::cout << VirtualStrategy::name << " making money for " << instrument_to_string<USDRUB>() << std::endl;
    }
};

template<typename INSTRUMENT_DEFINITION>
struct HasInstrument {
    template<typename STRATEGY>
    static constexpr bool check() {
        return STRATEGY::Instruments::template has<INSTRUMENT_DEFINITION>();
    }
};

enum class Environment {
    Arbitrage,
    MarketMaking
};

class MoexFxGateway {
public:
    constexpr static Venue VenueType = Venue::MOEX_FX;

    void connect() {};

    volatile MarketDataPackage readPackage() {
        return MarketDataPackage{USDRUB};
    };
};

class MoexEquityGateway {
public:
    constexpr static Venue VenueType = Venue::MOEX_EQUITY;

    void connect() {};

    volatile MarketDataPackage readPackage() {
        return MarketDataPackage{AFLT};
    };
};

class MoexFuturesGateway {
public:
    constexpr static Venue VenueType = Venue::MOEX_FUTURES;

    void connect() {};

    volatile MarketDataPackage readPackage() {
        return MarketDataPackage{USDRUB_F};
    };
};


template<typename T>
struct InstrumentToGatewayMapper {
    template<typename INSTRUMENT_DEFINITION>
    struct Mapper {
        using type = MoexFxGateway;
    };

    template<typename CUSTOM_TUPLE> requires is_not_empty_custom_tuple<CUSTOM_TUPLE>
    struct Mapper<CUSTOM_TUPLE> {
        using type = CUSTOM_TUPLE::template Map<InstrumentToGatewayMapper>;
    };

    template<typename INSTRUMENT_DEFINITION> requires (INSTRUMENT_DEFINITION::VenueType == Venue::MOEX_EQUITY)
    struct Mapper<INSTRUMENT_DEFINITION> {
        using type = MoexEquityGateway;
    };

    template<typename INSTRUMENT_DEFINITION> requires (INSTRUMENT_DEFINITION::VenueType == Venue::MOEX_FUTURES)
    struct Mapper<INSTRUMENT_DEFINITION> {
        using type = MoexFuturesGateway;
    };

    using type = Mapper<T>::type;
};

template<typename T>
struct StrategyToInstruments {
    template<typename STRATEGY>
    struct Mapper {
        using type = STRATEGY::Instruments;
    };

    using type = Mapper<T>::type;
};

template<typename STRATEGIES, Instrument INSTRUMENT, Venue VENUE>
void makeMoney(STRATEGIES &strategies, const MarketDataPackage &package) {
    using InstrumentType = InstrumentDefinition<INSTRUMENT, VENUE>;
    strategies.template applyIf<HasInstrument<InstrumentType>>
            ([&package]<typename STRATEGY>(STRATEGY &strategy) {
                strategy.template makeMoney<InstrumentType>(package);
            });
}

template<typename STRATEGIES>
void runMain() {
    using Gateways = STRATEGIES::template Map<StrategyToInstruments>::template Map<InstrumentToGatewayMapper>;
    STRATEGIES strategies{};
    Gateways gateways{};
    gateways.apply([]<typename GATEWAY>(GATEWAY &gateway) {
        gateway.connect();
    });

    /* while(true) */
    gateways.apply([&strategies]<typename GATEWAY>(GATEWAY &gateway) {
        MarketDataPackage package = gateway.readPackage();
        switch (package.instrument) {
            case USDRUB: {
                std::cout << "readPackage " << instrument_to_string<USDRUB>() << std::endl;
                makeMoney<STRATEGIES, USDRUB, Venue::MOEX_FX>(strategies, package);
                break;
            }
            case USDRUB_F: {
                std::cout << "readPackage " << instrument_to_string<USDRUB_F>() << std::endl;
                makeMoney<STRATEGIES, USDRUB_F, Venue::MOEX_FUTURES>(strategies, package);
                break;
            }
            case AFLT: {
                std::cout << "readPackage " << instrument_to_string<AFLT>() << std::endl;
                makeMoney<STRATEGIES, AFLT, Venue::MOEX_EQUITY>(strategies, package);
                break;
            }
        }
    });
}

template<typename T>
struct is_same_type {
    template<typename U>
    static constexpr bool check() {
        return std::is_same_v<T, U>;
    }
};

template<typename T>
struct MyMapper {
    template<typename U>
    struct Mapper {
        using type = long;
    };

    template<>
    struct Mapper<float> {
        using type = double;
    };

    using type = Mapper<T>::type;
};

int main() {
    std::cout << std::endl << "CustomTuple" << std::endl;
    {
        std::cout << std::endl << "Both Strategies" << std::endl;
        using Strategies = CustomTuple<ArbitrageStrategy, MarketMakingStrategy, MarketMakingStrategyFx>;
        runMain<Strategies>();
    }

    {
        std::cout << std::endl << "Arbitrage" << std::endl;
        using Strategies = CustomTuple<ArbitrageStrategy>;
        runMain<Strategies>();
    }

    {
        std::cout << std::endl << "MarketMaking" << std::endl;
        using Strategies = CustomTuple<MarketMakingStrategy>;
        runMain<Strategies>();
    }

    {
        std::cout << std::endl << "Both Strategies" << std::endl;
        using Strategies = CustomTuple<ArbitrageStrategy, MarketMakingStrategy>;
        runMain<Strategies>();
    }

    std::cout << std::endl << "AbstractStrategy" << std::endl;
    {
        const MarketDataPackage data(USDRUB);
        std::vector<AbstractStrategy> strategies{
                AbstractStrategy("MarketMakingStrategyRuntime", USDRUB_F, Venue::MOEX_FUTURES),
                AbstractStrategy("MarketMakingStrategyFxRuntime", USDRUB, Venue::MOEX_FX)};
        std::for_each(strategies.begin(), strategies.end(), [&data](auto &strategy) {
            strategy.makeMoney(data);
        });
    }

    std::cout << std::endl << "VirtualStrategy" << std::endl;
    {
        std::vector<std::unique_ptr<VirtualStrategy>> strategies{};
        strategies.push_back(std::make_unique<VirtualArbitrageStrategy>());
        strategies.push_back(std::make_unique<VirtualMarketMakingStrategy>());

        const MarketDataPackage data(USDRUB);
        std::for_each(strategies.begin(), strategies.end(), [&data](auto &strategy) {
            strategy->makeMoney(data);
        });
    }

    std::cout << std::endl << "CustomTuple Sample" << std::endl;
    using MyTuple = CustomTuple<int, float>;
    MyTuple tuple{};
    tuple.template set<int>(123);
    tuple.template set<float>(456.0f);
    std::cout << tuple.template get<int>() << std::endl;
    std::cout << tuple.template get<float>() << std::endl;

    tuple.apply([]<typename T>(T &value) {
        value *= 2;
    });

    std::cout << tuple.template get<int>() << std::endl;
    std::cout << tuple.template get<float>() << std::endl;

    tuple.template applyIf<is_same_type<int>>(
            []<typename T>(T &value) {
                value *= 2;
            });

    tuple.template applyIf<is_same_type<float>>(
            []<typename T>(T &value) {
                value *= 3;
            });

    std::cout << tuple.template get<int>() << std::endl;
    std::cout << tuple.template get<float>() << std::endl;

    using MappedCustomTuple = CustomTuple<int, float>::Map<MyMapper>;
    MappedCustomTuple mappedTuple{};
    mappedTuple.template set<long>(111);
    mappedTuple.template set<double>(333.0);
    std::cout << mappedTuple.template get<long>() << std::endl;
    std::cout << mappedTuple.template get<double>() << std::endl;

    //auto r = is_duplicate<int, float, CustomTuple<long, short>, char>;
    using MyTupleFlat = CustomTuple<short, MyTuple, double>;
    MyTupleFlat myTypleFlat{};
    myTypleFlat.template set<short>(1);
    myTypleFlat.template set<int>(2);
    myTypleFlat.template set<float>(3.f);
    myTypleFlat.template set<double>(666.0);
    myTypleFlat.template apply(
            []<typename T>(T &value) {
                std::cout << "flat: " << value << std::endl;
            });

    using MyTupleDedup = CustomTuple<int, CustomTuple<float, double, int>, float>;
    MyTupleDedup myTypleDedup{};
    myTypleDedup.template set<int>(555);
    myTypleDedup.template set<float>(666.f);
    myTypleDedup.template set<double>(777.f);
    myTypleDedup.template apply(
            []<typename T>(T &value) {
                std::cout << "deduplication: " << value << std::endl;
            });
    return 0;
}
