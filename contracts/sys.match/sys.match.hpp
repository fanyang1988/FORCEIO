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
#include <relay.token/relay.token.hpp>
#include <string>


namespace exchange {
   using namespace eosio;
   using std::string;
   using eosio::asset;
   using eosio::symbol_type;
   const account_name relay_token_acc = N(relay.token);
   const uint32_t INTERVAL_BLOCKS = /*172800*/ 24 * 3600 * 1000 / config::block_interval_ms;

   typedef double real_type;

   class exchange : public contract  {

   public:
      exchange(account_name self) : contract(self) {}

      void create(symbol_type base, name base_chain, symbol_type base_sym, symbol_type quote, name quote_chain, symbol_type quote_sym, uint32_t fee_rate, account_name exc_acc);

      void alter_pair_precision(symbol_type base, symbol_type quote);
      
      void alter_pair_fee_rate(symbol_type base, symbol_type quote, uint32_t fee_rate);
      
      void alter_pair_exc_acc(symbol_type base, symbol_type quote, account_name exc_acc);

      void match( account_name payer, account_name receiver, asset base, asset price, uint32_t bid_or_ask );
      
      void cancel(account_name maker, uint32_t type, uint64_t order_or_pair_id);
      
      void done(account_name exc_acc, name quote_chain, asset price, name base_chain, asset quantity, uint32_t bid_or_ask, time_point_sec timestamp);
      
      void done_helper(account_name exc_acc, name quote_chain, asset price, name base_chain, asset quantity, uint32_t bid_or_ask);
      
      void match_for_bid( account_name payer, account_name receiver, asset base, asset price);
      
      void match_for_ask( account_name payer, account_name receiver, asset base, asset price);
      
      void mark(name base_chain, symbol_type base_sym, name quote_chain, symbol_type quote_sym);
      
      void claim(name base_chain, symbol_type base_sym, name quote_chain, symbol_type quote_sym, account_name exc_acc, account_name fee_acc);
      
      void freeze(uint32_t id);
      
      void unfreeze(uint32_t id);
         
      asset calcfee(asset quant, uint64_t fee_rate);

      inline symbol_type get_pair_base( uint32_t pair_id ) const;
      inline symbol_type get_pair_quote( uint32_t pair_id ) const;
      inline account_name get_exchange_account( uint32_t pair_id ) const;
      inline uint32_t get_pair( name base_chain, symbol_type base_sym, name quote_chain, symbol_type quote_sym );
      inline uint32_t get_pair_id( name base_chain, symbol_type base_sym, name quote_chain, symbol_type quote_sym ) const;
      inline asset get_avg_price( uint32_t block_height, name base_chain, symbol_type base_sym, name quote_chain = {0}, symbol_type quote_sym = CORE_SYMBOL ) const;
      
      inline void upd_mark( name base_chain, symbol_type base_sym, name quote_chain, symbol_type quote_sym, asset sum, asset vol );

      struct pair {
          uint32_t id;
      
          name        base_chain;
          symbol_type base_sym;

          name        quote_chain;
          symbol_type quote_sym;
          
          uint32_t primary_key() const { return id; }
      };

   //private:
      struct trading_pair{
          uint32_t id;
          uint32_t pair_id;
          
          symbol_type base;
          name        base_chain;
          symbol_type base_sym;

          symbol_type quote;
          name        quote_chain;
          symbol_type quote_sym;
          
          uint32_t    fee_rate;
          account_name exc_acc;
          asset       fees_base;
          asset       fees_quote;
          uint32_t    frozen;
          
          uint32_t primary_key() const { return id; }
          uint128_t by_pair_sym() const { return (uint128_t(base.name()) << 64) | quote.name(); }
      };
      
      struct order {
         uint64_t        id;
         uint32_t        pair_id;
         account_name    maker;
         account_name    receiver;
         asset           base;
         asset           price;
         uint32_t        bid_or_ask;
         time_point_sec  timestamp;

         uint64_t primary_key() const { return id; }
         uint128_t by_pair_price() const { 
             //print("\n by_pair_price: order: id=", id, ", pair_id=", pair_id, ", bid_or_ask=", bid_or_ask,", base=", base,", price=", price,", maker=", maker, ", key=", (uint128_t(pair_id) << 96) | ((uint128_t)(bid_or_ask ? 1 : 0)) << 64 | (uint64_t)price.amount);
             return (uint128_t(pair_id) << 96) | ((uint128_t)(bid_or_ask ? 1 : 0)) << 64 | (uint64_t)price.amount; }
      };
      
      struct deal_info {
         uint32_t    id;
         uint32_t    pair_id;

         asset       sum;
         asset       vol;
         
         // [reset_block_height .. block_height_end]
         uint32_t    reset_block_height;// include 
         uint32_t    block_height_end;// include
         
         uint64_t primary_key() const { return id; }
         uint64_t by_pair_and_block_height() const {
            return (uint64_t(pair_id) << 32) | block_height_end;
         }
      };

      typedef eosio::multi_index<N(rawpairs), pair> raw_pairs;
      typedef eosio::multi_index<N(pairs), trading_pair,
         indexed_by< N(idxkey), const_mem_fun<trading_pair, uint128_t, &trading_pair::by_pair_sym>>
      > trading_pairs;    
      typedef eosio::multi_index<N(orderbook), order,
         indexed_by< N(idxkey), const_mem_fun<order, uint128_t, &order::by_pair_price>>
      > orderbook;    
      typedef eosio::multi_index<N(deals), deal_info,
         indexed_by< N(idxkey), const_mem_fun<deal_info, uint64_t, &deal_info::by_pair_and_block_height>>
      > deals;    

      void insert_order(
                       orderbook &orders, 
                       uint32_t pair_id, 
                       uint32_t bid_or_ask, 
                       asset base, 
                       asset price, 
                       account_name payer, 
                       account_name receiver);

      static asset to_asset( account_name code, name chain, symbol_type sym, const asset& a );
      static asset convert( symbol_type expected_symbol, const asset& a );
      static int64_t precision(uint64_t decimals)
      {
         int64_t p10 = 1;
         int64_t p = (int64_t)decimals;
         while( p > 0  ) {
            p10 *= 10; --p;
         }
         return p10;
      }
   };
   
   uint32_t exchange::get_pair( name base_chain, symbol_type base_sym, name quote_chain, symbol_type quote_sym ) {
      raw_pairs   raw_pairs_table(_self, _self);

      auto lower_key = std::numeric_limits<uint64_t>::lowest();
      auto lower = raw_pairs_table.lower_bound( lower_key );
      auto upper_key = std::numeric_limits<uint64_t>::max();
      auto upper = raw_pairs_table.upper_bound( upper_key );
      
      for ( auto itr = lower; itr != upper; ++itr ) {
         if (itr->base_chain == base_chain && itr->base_sym.name() == base_sym.name() && 
               itr->quote_chain == quote_chain && itr->quote_sym.name() == quote_sym.name()) {
            return itr->id;
         }
      }
      
      auto pk = raw_pairs_table.available_primary_key();
      raw_pairs_table.emplace( _self, [&]( auto& p ) {
         p.id = (uint32_t)pk;
         p.base_chain   = base_chain;
         p.base_sym     = base_sym;
         p.quote_chain  = quote_chain;
         p.quote_sym    = quote_sym;
      });

      return (uint32_t)pk;
   }
   
   /*void exchange::get_pair( name base_chain, symbol_type base_sym, name quote_chain, symbol_type quote_sym ) const {
      trading_pairs   trading_pairs_table(_self, _self);
     
      auto lower_key = std::numeric_limits<uint64_t>::lowest();
      auto lower = trading_pairs_table.lower_bound( lower_key );
      auto upper_key = std::numeric_limits<uint64_t>::max();
      auto upper = trading_pairs_table.upper_bound( upper_key );
      
      for ( auto itr = lower; itr != upper; ++itr ) {
         if (itr->base_chain == base_chain && itr->base_sym == base_sym && 
            itr->quote_chain == quote_chain && itr->quote_sym == quote_sym) 
             print("exchange::get_pair -- pair: id=", itr->id, "\n");
            return;
      }
          
      eosio_assert(false, "trading pair does not exist");
      
      return;
   }*/
   
   uint32_t exchange::get_pair_id( name base_chain, symbol_type base_sym, name quote_chain, symbol_type quote_sym ) const {
      raw_pairs   raw_pairs_table(_self, _self);

      auto lower_key = std::numeric_limits<uint64_t>::lowest();
      auto lower = raw_pairs_table.lower_bound( lower_key );
      auto upper_key = std::numeric_limits<uint64_t>::max();
      auto upper = raw_pairs_table.upper_bound( upper_key );
      
      for ( auto itr = lower; itr != upper; ++itr ) {
         if (itr->base_chain == base_chain && itr->base_sym.name() == base_sym.name() && 
               itr->quote_chain == quote_chain && itr->quote_sym.name() == quote_sym.name()) {
            print("exchange::get_pair_id -- pair: id=", itr->id, "\n");
            return itr->id;   
         }
      }
          
      eosio_assert(false, "raw pair does not exist");

      return 0;
   }
   
   symbol_type exchange::get_pair_base( uint32_t pair_id ) const {
      trading_pairs   trading_pairs_table(_self, _self);
     
      //auto itr1 = trading_pairs_table.find(pair_id);
      // eosio_assert(itr1 != trading_pairs_table.end(), "trading pair does not exist");
      
      auto walk_table_range = [&]( auto itr, auto end_itr ) {
         print("\n ---------------- begin to trading_pairs table: ----------------");
         for( ; itr != end_itr; ++itr ) {
             print("\n pair: id=", itr->id);
         }
         print("\n -------------------- walk through trading_pairs table ends ----------------:");
      };
      //auto lower_key = (uint128_t(itr1->id) << 96) | ((uint128_t)(bid_or_ask ? 0 : 1)) << 64 | std::numeric_limits<uint64_t>::lowest();
      auto lower_key = std::numeric_limits<uint64_t>::lowest();
      auto lower = trading_pairs_table.lower_bound( lower_key );
      //auto upper_key = (uint128_t(itr1->id) << 96) | ((uint128_t)(bid_or_ask ? 0 : 1)) << 64 | std::numeric_limits<uint64_t>::max();
      auto upper_key = std::numeric_limits<uint64_t>::max();
      auto upper = trading_pairs_table.upper_bound( upper_key );
      //walk_table_range(lower, upper);
      
      for( auto itr = lower; itr != upper; ++itr ) {
          print("\n pair: id=", itr->id);
          if (itr->id == pair_id) return itr->base;
      }
          
      eosio_assert(false, "trading pair does not exist");
      
      return 0;
   }
   
   symbol_type exchange::get_pair_quote( uint32_t pair_id ) const {
      trading_pairs   trading_pairs_table(_self, _self);
     
      //auto itr1 = trading_pairs_table.find(pair_id);
      // eosio_assert(itr1 != trading_pairs_table.end(), "trading pair does not exist");
      
      auto walk_table_range = [&]( auto itr, auto end_itr ) {
         print("\n ---------------- begin to trading_pairs table: ----------------");
         for( ; itr != end_itr; ++itr ) {
             print("\n pair: id=", itr->id);
         }
         print("\n -------------------- walk through trading_pairs table ends ----------------:");
      };
      //auto lower_key = (uint128_t(itr1->id) << 96) | ((uint128_t)(bid_or_ask ? 0 : 1)) << 64 | std::numeric_limits<uint64_t>::lowest();
      auto lower_key = std::numeric_limits<uint64_t>::lowest();
      auto lower = trading_pairs_table.lower_bound( lower_key );
      //auto upper_key = (uint128_t(itr1->id) << 96) | ((uint128_t)(bid_or_ask ? 0 : 1)) << 64 | std::numeric_limits<uint64_t>::max();
      auto upper_key = std::numeric_limits<uint64_t>::max();
      auto upper = trading_pairs_table.upper_bound( upper_key );
      //walk_table_range(lower, upper);
      
      for ( auto itr = lower; itr != upper; ++itr ) {
         print("\n pair: id=", itr->id);
         if (itr->id == pair_id) return itr->quote;
      }
          
      eosio_assert(false, "trading pair does not exist");
      
      return 0;
   }
   
   account_name exchange::get_exchange_account( uint32_t pair_id ) const {
      trading_pairs   trading_pairs_table(_self, _self);
     
      //auto itr1 = trading_pairs_table.find(pair_id);
      // eosio_assert(itr1 != trading_pairs_table.end(), "trading pair does not exist");
      
      auto walk_table_range = [&]( auto itr, auto end_itr ) {
         print("\n ---------------- begin to trading_pairs table: ----------------");
         for( ; itr != end_itr; ++itr ) {
             print("\n pair: id=", itr->id);
         }
         print("\n -------------------- walk through trading_pairs table ends ----------------:");
      };
      //auto lower_key = (uint128_t(itr1->id) << 96) | ((uint128_t)(bid_or_ask ? 0 : 1)) << 64 | std::numeric_limits<uint64_t>::lowest();
      auto lower_key = std::numeric_limits<uint64_t>::lowest();
      auto lower = trading_pairs_table.lower_bound( lower_key );
      //auto upper_key = (uint128_t(itr1->id) << 96) | ((uint128_t)(bid_or_ask ? 0 : 1)) << 64 | std::numeric_limits<uint64_t>::max();
      auto upper_key = std::numeric_limits<uint64_t>::max();
      auto upper = trading_pairs_table.upper_bound( upper_key );
      //walk_table_range(lower, upper);
      
      for ( auto itr = lower; itr != upper; ++itr ) {
         print("\n pair: id=", itr->id);
         if (itr->id == pair_id) return itr->exc_acc;
      }
          
      eosio_assert(false, "trading pair does not exist");
      
      return 0;
   }
   
   /*
   block_height: end block height
   */
   asset exchange::get_avg_price( uint32_t block_height, name base_chain, symbol_type base_sym, name quote_chain, symbol_type quote_sym ) const {
      deals   deals_table(_self, _self);
      asset   avg_price = asset(0, quote_sym);

      uint32_t pair_id = 0xFFFFFFFF;
      
      raw_pairs   raw_pairs_table(_self, _self);

      auto lower_key = std::numeric_limits<uint64_t>::lowest();
      auto lower = raw_pairs_table.lower_bound( lower_key );
      auto upper_key = std::numeric_limits<uint64_t>::max();
      auto upper = raw_pairs_table.upper_bound( upper_key );
      auto itr = lower;
      
      for ( itr = lower; itr != upper; ++itr ) {
         if (itr->base_chain == base_chain && itr->base_sym.name() == base_sym.name() && 
               itr->quote_chain == quote_chain && itr->quote_sym.name() == quote_sym.name()) {
            print("exchange::get_avg_price -- pair: id=", itr->id, "\n");
            pair_id = itr->id;
            break;
         }
      }
      if (itr == upper) {
         print("exchange::get_avg_price: trading pair not exist! base_chain=", base_chain.to_string().c_str(), ", base_sym=", base_sym, ", quote_chain", quote_chain.to_string().c_str(), ", quote_sym=", quote_sym, "\n");
         return avg_price;
      }
      
      lower_key = ((uint64_t)pair_id << 32) | 0;
      auto idx_deals = deals_table.template get_index<N(idxkey)>();
      auto itr1 = idx_deals.lower_bound(lower_key);
      if (!(itr1 != idx_deals.end() && itr1->pair_id == pair_id)) {
         print("exchange::get_avg_price: trading pair not marked!\n");
         return avg_price;
      }
      
      lower_key = ((uint64_t)pair_id << 32) | block_height;
      itr1 = idx_deals.lower_bound(lower_key);
      if (itr1 == idx_deals.cend()) itr1--;

      if (itr1->vol.amount > 0 && block_height >= itr1->reset_block_height) 
         avg_price = itr1->sum * precision(itr1->vol.symbol.precision()) / itr1->vol.amount;
      /*print("exchange::get_avg_price pair_id=", itr1->pair_id, ", block_height=", block_height, 
         ", reset_block_height=", itr1->reset_block_height, ", block_height_end=", itr1->block_height_end, 
         ", sum=", itr1->sum, ", vol=", itr1->vol, ", avg_price=", avg_price,"\n");*/
      return avg_price;
   }
 
   void exchange::upd_mark( name base_chain, symbol_type base_sym, name quote_chain, symbol_type quote_sym, asset sum, asset vol ) {
      deals   deals_table(_self, _self);
      
      auto pair_id = get_pair_id(base_chain, base_sym, quote_chain, quote_sym);
     
      auto lower_key = ((uint64_t)pair_id << 32) | 0;
      auto idx_deals = deals_table.template get_index<N(idxkey)>();
      auto itr1 = idx_deals.lower_bound(lower_key);
      if (!( itr1 != idx_deals.end() && itr1->pair_id == pair_id )) {
         print("exchange::upd_mark trading pair not marked!\n");
         return;
      }
      
      uint32_t curr_block = current_block_num();
      lower_key = ((uint64_t)pair_id << 32) | curr_block;
      itr1 = idx_deals.lower_bound(lower_key);
      if (itr1 == idx_deals.cend()) itr1--;
      if ( curr_block <= itr1->block_height_end ) {
         idx_deals.modify( itr1, _self, [&]( auto& d ) {
            d.sum += sum;
            d.vol += vol;
         });
      } else {
         auto start_block =  itr1->reset_block_height + (curr_block - itr1->reset_block_height) / INTERVAL_BLOCKS * INTERVAL_BLOCKS;
         auto pk = deals_table.available_primary_key();
         deals_table.emplace( _self, [&]( auto& d ) {
            d.id                 = (uint32_t)pk;
            d.pair_id            = pair_id;
            d.sum                = sum;
            d.vol                = vol;
            d.reset_block_height = start_block;
            d.block_height_end   = start_block + INTERVAL_BLOCKS - 1;
         });   
      }
      
      // test
      /*{
         name test_base_chain; test_base_chain.value = N(test1);
         symbol_type test_base_sym = S(2, TESTA);
         name test_quote_chain; test_quote_chain.value = N(test2);
         symbol_type test_quote_sym = S(2, TESTB);
         
         get_avg_price( curr_block, test_base_chain, test_base_sym, test_quote_chain, test_quote_sym );
         get_avg_price( curr_block, base_chain, base_sym, quote_chain, quote_sym );
         get_avg_price( curr_block-10, base_chain, base_sym, quote_chain, quote_sym );
         get_avg_price( curr_block+10, base_chain, base_sym, quote_chain, quote_sym );
      }*/
      
      return ;
   }
}
#endif //EOSIO_EXCHANGE_H
