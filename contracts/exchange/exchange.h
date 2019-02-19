//
// Created by root1 on 18-7-26.
//

#ifndef EOSIO_EXCHANGE_H
#define EOSIO_EXCHANGE_H

#pragma once


//#include <eosio.system/native.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/privileged.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/eosio.hpp>
#include <force.token/force.token.hpp>
#include <string>


namespace exchange {
    using namespace eosio;
    using std::string;
    using eosio::asset;
    using eosio::symbol_type;

    typedef double real_type;

    class exchange : public contract  {

    public:
        exchange(account_name self) : contract(self) {}

        void create(symbol_type base, symbol_type quote);

        void trade( account_name payer, asset base, asset price, uint32_t bid_or_ask);


    private:
        class symbol {
           public:
        
              static constexpr uint8_t max_precision = 18;
        
              explicit symbol(uint8_t p, const char* s): m_value(string_to_symbol(p, s)) {
                 eosio_assert(valid(), "invalid symbol");
              }
              explicit symbol(uint64_t v = CORE_SYMBOL): m_value(v) {
                 eosio_assert(valid(), "invalid symbol");
              }
              explicit symbol(symbol_type s): m_value(s.value) {
                 eosio_assert(valid(), "invalid symbol");
              }
              
              uint64_t value() const { return m_value; }
              bool valid() const
              {
                 const auto& s = name();
                 return decimals() <= max_precision;
              }
        
              uint8_t decimals() const { return m_value & 0xFF; }
              uint64_t precision() const
              {
                 eosio_assert( decimals() <= max_precision, "precision should be <= 18" );
                 uint64_t p10 = 1;
                 uint64_t p = decimals();
                 while( p > 0  ) {
                    p10 *= 10; --p;
                 }
                 return p10;
              }
              string name() const
              {
                 uint64_t v = m_value;
                 v >>= 8;
                 string result;
                 while (v > 0) {
                    char c = v & 0xFF;
                    result += c;
                    v >>= 8;
                 }
                 return result;
              }
        
           private:
              uint64_t m_value;
        }; // class symbol

        struct trading_pair{
            uint32_t id;
       
            symbol_type base;
            symbol_type quote;
            
            uint32_t primary_key() const { return id; }
            uint128_t by_pair_sym() const { return (uint128_t(base.name()) << 64) | quote.name(); }
        };
        
        struct order {
            uint64_t        id;
            uint32_t        pair_id;
            account_name    maker;
            asset           base;
            asset           price;
            uint32_t        bid_or_ask;

            uint64_t primary_key() const { return id; }
            uint128_t by_pair_price() const { 
                print("\n by_pair_price: order: id=", id, ", pair_id=", pair_id, ", bid_or_ask=", bid_or_ask,", base=", base,", price=", price,", maker=", maker, ", key=", (uint128_t(pair_id) << 96) | ((uint128_t)(bid_or_ask ? 1 : 0)) << 64 | price.amount);
                return (uint128_t(pair_id) << 96) | ((uint128_t)(bid_or_ask ? 1 : 0)) << 64 | price.amount; }
        };
/* 
   ordered_unique<tag<by_code_scope_table>,
            composite_key< table_id_object,
               member<table_id_object, account_name, &table_id_object::code>,
               member<table_id_object, scope_name,   &table_id_object::scope>,
               member<table_id_object, table_name,   &table_id_object::table>
            >
         >      
*/
        typedef eosio::multi_index<N(pairs), trading_pair,
           indexed_by< N(idxkey), const_mem_fun<trading_pair, uint128_t,  &trading_pair::by_pair_sym>>
        > trading_pairs;    
        typedef eosio::multi_index<N(orderbook), order,
           indexed_by< N(idxkey), const_mem_fun<order, uint128_t,  &order::by_pair_price>>
        > orderbook;    

        asset to_asset( account_name code, const asset& a );
        asset convert( symbol_type expected_symbol, const asset& a );
        uint64_t precision(uint8_t decimals) const
        {
           eosio_assert( decimals >= 0, "precision should be >= 0" );
           uint64_t p10 = 1;
           uint64_t p = decimals;
           while( p > 0  ) {
              p10 *= 10; --p;
           }
           return p10;
        }
    };
}
#endif //EOSIO_EXCHANGE_H