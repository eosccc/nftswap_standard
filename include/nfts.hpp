#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>


#define max(a,b) (a>b?a:b)

namespace eosiosystem {
   class system_contract;
}

namespace eosio {

   using std::string;

   /**
    * eosio.token contract defines the structures and actions that allow users to create, issue, and manage
    * tokens on EOSIO based blockchains.
    */
   class [[eosio::contract("nfts")]] nfts : public contract {
      public:
         using contract::contract;

         /**
          * Allows `creator` account to create a token in supply of `maximum_supply`. If validation is successful a new entry in statstable for token symbol scope gets created.
          *
          * @param creator - the account that creates the token,
          * @param maximum_supply - the maximum supply set for the token created.
          *
          * @pre Token symbol has to be valid,
          * @pre Token symbol must not be already created,
          * @pre maximum_supply has to be smaller than the maximum supply allowed by the system: 1^62 - 1.
          * @pre Maximum supply must be positive;
          */
         [[eosio::action]]
         void init( const name&   creator,const asset&  maximum_supply,const std::string&	token_title,const std::string&	token_logo,const std::string&	categories);
                      
         [[eosio::action]]
         void create( const name&    creator,const name&    owner,name genes,std::string token_name,std::string image_url,std::string mdata,const uint64_t& level,const uint64_t& category,const asset& nft_value);

         [[eosio::action]]
         void burn(const name& owner,   const uint64_t& id);

         [[eosio::action]]
         void transfer(const uint64_t& id, const name& from, const name& to, const std::string& memo);

         using init_action = eosio::action_wrapper<"init"_n, &nfts::init>;
         using create_action = eosio::action_wrapper<"create"_n, &nfts::create>;
         using burn_action = eosio::action_wrapper<"burn"_n, &nfts::burn>;
         using transfer_action = eosio::action_wrapper<"transfer"_n, &nfts::transfer>;

      private:
         struct [[eosio::table]] nft_stats {
            name     creator;
            asset    supply;
            asset    max_supply;
            uint64_t	token_count;
        std::string	token_title;
        std::string	token_logo;
        std::string	categories;

            uint64_t primary_key()const { return creator.value; }
         };

         struct [[eosio::table]] account {
            name owner;
            std::vector<uint64_t> nft_ids;

            uint64_t primary_key()const { return owner.value; }
         };

         struct [[eosio::table]] token {
            uint64_t id;
            name    creator;
            name    owner;
            name genes;
        std::string token_name;
		  std::string token_summary;
        std::string image_url;
		  std::string mdata; // mutable data
            uint64_t	level;
            uint64_t category;
		      asset	  nft_value;
            time_point_sec	create_time;  

            uint64_t primary_key()const { return id; }
            uint64_t get_secondary_1() const { return creator.value;}
            uint64_t get_secondary_2() const { return owner.value;}
            uint64_t get_secondary_3() const { return level;}
	         uint64_t get_secondary_4() const { return category;}
            uint64_t get_secondary_5() const { return nft_value.amount;}
         };

         typedef eosio::multi_index< "stat"_n, nft_stats > stats;
         typedef eosio::multi_index< "accounts"_n, account > accounts;
         typedef eosio::multi_index<"tokens"_n, token, 
         indexed_by<"bycreator"_n, const_mem_fun<token, uint64_t, &token::get_secondary_1>>,
        indexed_by<"byowner"_n, const_mem_fun<token, uint64_t, &token::get_secondary_2>>,
        indexed_by<"bylevel"_n, const_mem_fun<token, uint64_t, &token::get_secondary_3>>,
	     indexed_by<"bycategory"_n, const_mem_fun<token, uint64_t, &token::get_secondary_4>>,
	     indexed_by<"bynftval"_n, const_mem_fun<token, uint64_t, &token::get_secondary_5>>
	     > tokens;

		 void sub_tokenid( const name&  creator, const name& owner, const uint64_t& id );
		 void add_token( const name&    creator,const name& owner, name genes,std::string token_name,std::string image_url,std::string mdata,const uint64_t& level,const uint64_t& category,const asset& nft_value);
         void add_tokenid( const name&    creator,const name& owner,const uint64_t& id, const name& ram_payer );

   };

}
