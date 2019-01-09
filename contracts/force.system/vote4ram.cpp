#include "force.system.hpp"

namespace eosiosystem {
    void system_contract::vote4ram( const account_name voter, const account_name bpname, const asset stake ) {
      require_auth(voter);

      bps_table bps_tbl(_self, _self);
      const auto& bp = bps_tbl.get(bpname, "bpname is not registered");

      eosio_assert(stake.symbol == CORE_SYMBOL, "only support CORE SYMBOL token");
      eosio_assert(0 <= stake.amount && stake.amount % 10000 == 0,
                   "need stake quantity >= 0 and quantity is integer");

      int64_t change = 0;
      votes4ram_table votes_tbl(_self, voter);
      auto vts = votes_tbl.find(bpname);
      if( vts == votes_tbl.end()) {
         change = stake.amount;
         votes_tbl.emplace(voter, [&]( vote_info& v ) {
            v.bpname = bpname;
            v.staked = stake;
         });
      } else {
         change = stake.amount - vts->staked.amount;
         votes_tbl.modify(vts, 0, [&]( vote_info& v ) {
            v.voteage += v.staked.amount / 10000 * ( current_block_num() - v.voteage_update_height );
            v.voteage_update_height = current_block_num();
            v.staked = stake;
            if( change < 0 ) {
               v.unstaking.amount += -change;
               v.unstake_height = current_block_num();
            }
         });
      }
      eosio_assert(bp.isactive || (!bp.isactive && change < 0), "bp is not active");
      if( change > 0 ) {
         INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {voter, N(active)},
                                                       { voter, N(eosio), asset(change), std::string("vote4ram") } );
      }

      bps_tbl.modify(bp, 0, [&]( bp_info& b ) {
         b.total_voteage += b.total_staked * ( current_block_num() - b.voteage_update_height );
         b.voteage_update_height = current_block_num();
         b.total_staked += change / 10000;
      });

      vote4ramsum_table vote4ramsum_tbl(_self, _self);
      auto vtss = vote4ramsum_tbl.find(voter);
      if(vtss == vote4ramsum_tbl.end()){
         vote4ramsum_tbl.emplace(voter, [&]( vote4ram_info& v ) {
            v.voter = voter;
            v.staked = stake; // for first vote all staked is stake
         });
      }else{
         vote4ramsum_tbl.modify(vtss, 0, [&]( vote4ram_info& v ) {
            v.staked += asset{change};
         });
      }

      set_need_check_ram_limit( voter );
   }

   void system_contract::unfreezeram( const account_name voter, const account_name bpname ) {
      require_auth(voter);

      votes4ram_table votes_tbl(_self, voter);
      const auto& vts = votes_tbl.get(bpname, "voter have not add votes to the the producer yet");

      eosio_assert(vts.unstake_height + FROZEN_DELAY < current_block_num(), "unfreeze is not available yet");
      eosio_assert(0 < vts.unstaking.amount, "need unstaking quantity > 0");

      INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {N(eosio), N(active)},
                                                    { N(eosio), voter, vts.unstaking, std::string("unfreezeram") } );

      votes_tbl.modify(vts, 0, [&]( vote_info& v ) {
         v.unstaking.set_amount(0);
      });
   }

}