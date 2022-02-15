#include <nfts.hpp>

namespace eosio {

void nfts::init( const name&   creator,const asset&  maximum_supply,const std::string&	token_title,const std::string&	token_logo,const std::string&	categories)
{
    require_auth( get_self());

    auto sym = maximum_supply.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( maximum_supply.is_valid(), "invalid supply");
    check( maximum_supply.amount > 0, "max-supply must be positive");
    check( token_title.size() <= 256, "token-title has more than 256 bytes" );
    check( is_account( creator ), "creator account does not exist");

    stats statstable( get_self(), get_self().value);
    auto existing = statstable.find( creator.value);
    check( existing == statstable.end(), "creator already exists" );

    statstable.emplace( get_self(), [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply    = maximum_supply;
       s.creator       = creator;
       s.token_count   = 0;
       s.token_title   = token_title;
	   s.token_logo    = token_logo;
       s.categories    = categories;
    });
}


void nfts::create( const name&    creator,const name&    owner,name genes,std::string token_name,std::string image_url,std::string mdata,const uint64_t& level,const uint64_t& category,const asset& nft_value)
{
    auto sym = nft_value.symbol;
    check( sym.is_valid(), "无效代币符号 invalid symbol name" );
    //check( genes.size() <= 32, "genes has more than 32 bytes" );
    check( token_name.size() <= 256, "token-name has more than 256 bytes" );
	check( is_account( owner ), "owner account does not exist");

    stats statstable( get_self(), get_self().value);
    auto existing = statstable.find( creator.value);
    check( existing != statstable.end(), "creator does not exist, init before create" );
    const auto& st = *existing;

    require_auth( st.creator );
    check( nft_value.is_valid(), "invalid nft-value" );
    check( nft_value.amount > 0, "must create positive nft-value" );

    check( nft_value.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( nft_value.amount <= st.max_supply.amount - st.supply.amount, "nft-value exceeds available supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.supply += nft_value;
       s.token_count += 1;
    });

    add_token( creator, owner, genes, token_name,image_url,mdata,level,category,nft_value );
}



void nfts::burn(const name& owner,   const uint64_t& id) 
{
	check( is_account( owner ), "owner account does not exist");
    check(id > 0, "The id must be positive");
	
	tokens token_acnts( get_self(), get_self().value );
    auto t_itr = token_acnts.require_find(id, "Token does not exist");
	
    require_auth(t_itr->creator);

    stats statstable( get_self(), get_self().value);
    const auto& st = statstable.require_find( t_itr->creator.value, "creator does not exist");
    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.supply -= t_itr->nft_value;
       s.token_count -= 1;
    });

	sub_tokenid( t_itr->creator, owner, id );
	token_acnts.erase( t_itr );

    
}



void nfts::transfer( const uint64_t& id,const name&  from,const name& to, const string&  memo )
{
    check( from != to, "cannot transfer to self" );
    require_auth( from );
    //return;
    check( is_account( to ), "to account does not exist");
    check(id >= 0, "The id must be positive");
	
	
    tokens ttable( get_self(), get_self().value );
    //const auto& st = ttable.get( id );
	const auto t_itr = ttable.require_find(id, "Token does not exist");
	check( from == t_itr->owner, "Token does not exist" );
	check( to != t_itr->owner, "Token aleady exist" );
	

    
    require_recipient( from );
    require_recipient( to );

    check( memo.size() <= 256, "memo has more than 256 bytes" );

    //auto payer = has_auth( to ) ? to : from;

    

    sub_tokenid( t_itr->creator, from, id );
    add_tokenid( t_itr->creator, to, id, from );
	
	ttable.modify( t_itr, same_payer, [&]( auto& a ) {
        a.owner = to;
      });
	
}

void nfts::sub_tokenid( const name&  creator, const name& owner, const uint64_t& id ) {
   accounts a_acnts( get_self(), creator.value );
   const auto& ac = a_acnts.require_find( owner.value, "Owner does not exist");
   
   std::vector<uint64_t> nft_ids = ac->nft_ids;

    auto id_itr = std::find(nft_ids.begin(), nft_ids.end(), id);

    check(id_itr != nft_ids.end(),
        "The id does not exist");
    nft_ids.erase(id_itr);
   
   if(nft_ids.empty()){a_acnts.erase( ac );}
   else {
	   a_acnts.modify( ac, same_payer, [&]( auto& a ) {
         a.nft_ids = nft_ids;
      });
   };

   
}

void nfts::add_token( const name&    creator, const name& owner, name genes,std::string token_name,std::string image_url,std::string mdata,const uint64_t& level,const uint64_t& category,const asset& nft_value)
{
   check( is_account( owner ), "owner account does not exist");

   //require_auth( creator);
   tokens token_acnts( get_self(), get_self().value );
   token_acnts.emplace( creator, [&]( auto& t ) {
          t.id         = max(1001, token_acnts.available_primary_key());
          t.creator    = creator;
          t.owner      = owner;
          t.genes      = genes;
          t.token_name = token_name;
          t.image_url  = image_url;
          t.mdata      = mdata;
          t.level      = level;
          t.category   = category;
          t.nft_value  = nft_value;
          t.create_time= time_point_sec(current_time_point().sec_since_epoch() );
          add_tokenid(creator,owner,t.id,creator);
      });

}

void nfts::add_tokenid( const name&    creator,const name& owner,const uint64_t& id, const name& ram_payer )
{
   //require_auth( creator);
   accounts a_acnts( get_self(), creator.value );
   auto ac = a_acnts.find( owner.value  );
   
   //vector<uint64_t>  vid(id);
   if( ac == a_acnts.end() ) {
	   std::vector<uint64_t> nft_ids = {id};
      a_acnts.emplace( ram_payer, [&]( auto& a ){
        a.owner = owner;
		a.nft_ids = nft_ids;
      });
   } else {
	   std::vector<uint64_t>  nft_ids = ac->nft_ids;
       check(std::find(nft_ids.begin(), nft_ids.end(), id) == nft_ids.end(),
        "The id is already exist");
       nft_ids.push_back(id);
      a_acnts.modify( ac, ram_payer, [&]( auto& a ) {
		a.nft_ids = nft_ids;
      });
   }
}


} /// namespace eosio
