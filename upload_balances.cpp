/*
 * Copyright (c) 2017 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <graphene/upload_balances/upload_balances.hpp>

#include <graphene/app/impacted.hpp>

#include <graphene/chain/market_object.hpp>

#include <graphene/chain/account_evaluator.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/config.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/chain/transaction_evaluation_state.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/thread/thread.hpp>

#include <curl/curl.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string.hpp>
#include <regex>

#include <iostream>

using namespace fc;
namespace graphene { namespace upload_balances {
//void elasticsearch_plugin_impl::sendBulk(std::string _elasticsearch_node_url, bool _elasticsearch_logs)
//{
//
//   // curl buffers to read
//   std::string readBuffer;
//   std::string readBuffer_logs;
//
//   std::string bulking = "";
//
//   bulking = boost::algorithm::join(bulk, "\n");
//   bulking = bulking + "\n";
//   bulk.clear();
//
//   //wlog((bulking));
//
   
//   std::string url = _elasticsearch_node_url + "_bulk";
//   curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//   curl_easy_setopt(curl, CURLOPT_POST, true);
//   curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//   curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bulking.c_str());
//   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//   curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&readBuffer);
//   curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcrp/0.1");
//   //curl_easy_setopt(curl, CURLOPT_VERBOSE, true);
//   curl_easy_perform(curl);
//
//   long http_code = 0;
//   curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
//   if(http_code == 200) {
//      // all good, do nothing
//   }
//   else if(http_code == 429) {
//      // repeat request?
//   }
//   else {
//      // exit everything ?
//   }
//
//   if(_elasticsearch_logs) {
//      auto logs = readBuffer;
//      // do logs
//      std::string url_logs = _elasticsearch_node_url + "logs/data/";
//      curl_easy_setopt(curl, CURLOPT_URL, url_logs.c_str());
//      curl_easy_setopt(curl, CURLOPT_POST, true);
//      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, logs.c_str());
//      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &readBuffer_logs);
//      curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcrp/0.1");
//      //curl_easy_setopt(curl, CURLOPT_VERBOSE, true);
//      //ilog("log here curl: ${output}", ("output", readBuffer_logs));
//      curl_easy_perform(curl);
//
    
//   }
//}
    static fc::thread getline_thread("upload_balances");
#define UPLOAD_BALANCES_POST_WAIT_TIME "upload-balances-post-wait-time"
#define UPLOAD_BALANCES_HOUR_PER_DAY "upload-balances-hour-per-day"
#define UPLOAD_BALANCES_MINUTE_PER_DAY "upload-balances-minute-per-day"
#define UPLOAD_BALANCES_ACCOUNT_BALANCE_URL "upload-balances-account-balance-url"
#define UPLOAD_BALANCES_ACCOUNT_BALANCE_WRITE_TO_FILE_BALANCE_NAME "upload-balances-account-balance-write-to-file-balance-name"
#define UPLOAD_BALANCES_ACCOUNT_BALANCE_SEND_MAX_COUNT "upload-balances-account-balance-send-max-count"
#define UPLOAD_BALANCES_BORROW_URL "upload-balances-borrow-url"
#define UPLOAD_BALANCES_ACCOUNT_BALANCE_WRITE_TO_FILE_CALL_ORDERS_NAME "upload-balances-account-balance-write-to-file-call-orders-name"
#define UPLOAD_BALANCES_BORROW_ASSETS "upload-balances-borrow-assets"

    
    
    void upload_balances::plugin_set_program_options(
                                                 boost::program_options::options_description& command_line_options,
                                                 boost::program_options::options_description& config_file_options)
    {
        command_line_options.add_options()
        (UPLOAD_BALANCES_POST_WAIT_TIME,boost::program_options::value<uint32_t>(),"POST请求等待响应时间(秒) 不设置默认为45秒")
        (UPLOAD_BALANCES_HOUR_PER_DAY, boost::program_options::value<uint32_t>(), "每天的几点(以0时区为准)上传")
        (UPLOAD_BALANCES_MINUTE_PER_DAY, boost::program_options::value<uint32_t>(), "每天的几分(以0时区为准)上传")
        (UPLOAD_BALANCES_ACCOUNT_BALANCE_URL,boost::program_options::value<string>(), "获取全部账户余额POST请求发送地址")
        (UPLOAD_BALANCES_ACCOUNT_BALANCE_WRITE_TO_FILE_BALANCE_NAME,boost::program_options::value<string>(),"当获取账户余额请求发送失败时,文件写入地址")
        (UPLOAD_BALANCES_ACCOUNT_BALANCE_SEND_MAX_COUNT,boost::program_options::value<int>(),"每次post请求传递最大账户余额数量")
        (UPLOAD_BALANCES_BORROW_URL,boost::program_options::value<string>(),"上传账户借入地址(若不需上传该字段注释掉)")
        (UPLOAD_BALANCES_BORROW_ASSETS,boost::program_options::value<string>(),"需要获取借入的币种数组(json array)")
        (UPLOAD_BALANCES_ACCOUNT_BALANCE_WRITE_TO_FILE_CALL_ORDERS_NAME,boost::program_options::value<string>(),"当获取全部借入请求发送失败时,文件写入地址")
        ;
        config_file_options.add(command_line_options);
    }
    
    std::string upload_balances::plugin_name()const
    {
        return "upload_balances";
    }
    
    std::string upload_balances::plugin_description()const
    {
        return "upload_balances ";
    }
    
    void upload_balances::plugin_initialize(const boost::program_options::variables_map& options) {
        try {
            ilog("upload_balances plugin: plugin_initialize() begin");
            
            if( options.count("plugins") )
            {
                std::vector<string> wanted;
                boost::split(wanted, options.at("plugins").as<std::string>(), [](char c){return c == ' ';});
                
                auto itr = wanted.begin();
                
                bool contain = false;
                
                while (itr != wanted.end()) {
                    string name = *itr;
                    
                    if (name == "market_history") {
                        contain = true;
                        break;
                    }
                    
                    itr++;
                }
                
                FC_ASSERT(contain,"upload_balance 需要 market_history plugin !!!请在config.ini 中添加对应plugin 或在命令行中添加");
                
            }
//            if (options.)
            if( options.count(UPLOAD_BALANCES_POST_WAIT_TIME) ) {
                post_wait_time = options[UPLOAD_BALANCES_POST_WAIT_TIME].as<uint32_t>();
            }
            
            if( options.count(UPLOAD_BALANCES_HOUR_PER_DAY) ) {
                hour = options[UPLOAD_BALANCES_HOUR_PER_DAY].as<uint32_t>();
            }
            
            if( options.count(UPLOAD_BALANCES_MINUTE_PER_DAY) ) {
                minute = options[UPLOAD_BALANCES_MINUTE_PER_DAY].as<uint32_t>();
            }
            
            if (options.count(UPLOAD_BALANCES_ACCOUNT_BALANCE_URL)) {
                balance_url = options[UPLOAD_BALANCES_ACCOUNT_BALANCE_URL].as<string>();
            }
            
            if (options.count(UPLOAD_BALANCES_ACCOUNT_BALANCE_WRITE_TO_FILE_BALANCE_NAME)) {
                write_to_file_balance_name = options[UPLOAD_BALANCES_ACCOUNT_BALANCE_WRITE_TO_FILE_BALANCE_NAME].as<string>();
            }
            
            if (options.count(UPLOAD_BALANCES_ACCOUNT_BALANCE_WRITE_TO_FILE_CALL_ORDERS_NAME)) {
                write_to_file_call_order_name = options[UPLOAD_BALANCES_ACCOUNT_BALANCE_WRITE_TO_FILE_CALL_ORDERS_NAME].as<string>();
            }
            
            if (options.count(UPLOAD_BALANCES_ACCOUNT_BALANCE_SEND_MAX_COUNT)) {
                max_count = options[UPLOAD_BALANCES_ACCOUNT_BALANCE_SEND_MAX_COUNT].as<int>();
            }
            
            if (options.count(UPLOAD_BALANCES_BORROW_URL)) {
                borrow_url = options[UPLOAD_BALANCES_BORROW_URL].as<string>();
            }
            if (options.count(UPLOAD_BALANCES_BORROW_ASSETS)) {
                auto seeds_str = options.at(UPLOAD_BALANCES_BORROW_ASSETS).as<string>();
                upload_assets = fc::json::from_string(seeds_str).as<vector<string>>();
            }
            
            ilog("upload_balances plugin: plugin_initialize() end");
        } FC_LOG_AND_RETHROW()
        
        return;
    }
    
    void upload_balances::plugin_startup() {
        // 注册事件回调函数
        
        start_schedule();
    
        return;
    }
    
    
    
    void upload_balances::start_schedule() {
        fc::time_point now = fc::time_point::now();
        
        string time_string = fc::string(now);
        
        std::vector<string> wanted;
        boost::split(wanted, time_string, [](char c){return c == 'T';});
        
        string year_day = wanted[0];
        
        char buff[100];
        snprintf(buff, sizeof(buff), "%02d:%02d:00", hour,minute);
        
        string hours = buff;
        
        string str = year_day+ "T" + hours;
        
        ilog(str);
        
        fc::time_point time_point = fc::time_point::from_iso_string(year_day+ "T" + hours);
        
        if (now > time_point) {
            time_point += fc::days(1);
        }
        
        ilog("Start at time:${time}",("time",fc::string(time_point)));
        
        getline_thread.schedule([&](){
            
            dlog("准备发送账户余额数据");
            this->send_data_upload_account_balance();
            dlog("账户余额发送完毕");
            
            dlog("准备发送借入数据");
            
            if (upload_assets.size() > 0) {
                const auto& assets_by_symbol = database().get_index_type<asset_index>().indices().get<by_symbol>();
                
                auto itr_asset = upload_assets.begin();
                
                while (itr_asset != upload_assets.end()) {
                    string symbol_or_id = *itr_asset;
                    dlog("发送借入数据 开始:${asset}",("asset",symbol_or_id));
                    try {
                        if( !symbol_or_id.empty() && std::isdigit(symbol_or_id[0]) )
                        {
                            auto ptr = database().find(variant(symbol_or_id).as<asset_id_type>());
                            asset_object asset = *ptr;
                            send_data_upload_asset_call_orders(asset);
                        }else {
                            auto ptr = assets_by_symbol.find(symbol_or_id);
                            asset_object asset = *ptr;
                            send_data_upload_asset_call_orders(asset);
                        }
                        dlog("发送借入数据 结束:${asset}",("asset",symbol_or_id));
                    } catch (...) {
                        elog("发送借入数据 获取失败:${asset}",("asset",symbol_or_id));
                    }
                    itr_asset++;
                }
            }
            
            dlog("借入数据发送完毕");
            
            this->start_schedule();
        }, time_point);
    }
    
    void upload_balances::plugin_shutdown() {
        // 卸载事件回调函
        return;
    }
    
    void upload_balances::send_data_upload_account_balance() {
        if (balance_url.empty()) return;
        
        long long value = 6;
        
        fc::time_point now = fc::time_point::now();
        
        int upload_per_times = max_count;
        
        ofstream out(write_to_file_balance_name,ios::app);
        
        while (true) {
            vector<mutable_variant_object> vector;
            
            for (int i = 0; i < upload_per_times; i ++) {
                try {
                    account_id_type id(value);
                    
                    account_object account = id(database());
                    
                    mutable_variant_object variants("account_id",id);
                    
                    variants.set("account_name", account.name);
                    
                    variants.set("account_balances", this->get_account_balances(id));
                    
                    vector.push_back(variants);
        
                    value ++;
                } catch (...) {
                    break;
                }
            }
            
            string json = fc::json::to_string(vector);
            
            bool result = send_json_string(balance_url,json);
            
            if (!result) out<<json<<endl;
            
            if (vector.size() < upload_per_times) {
                break;
            }
        }
        
        out.close();
        
        fc::time_point last_data = fc::time_point::now();
        fc::microseconds second = last_data - now;
        dlog("发送余额共计毫秒:${second}",("second",second.count() / 100));
    }
    
    void upload_balances::send_data_upload_asset_call_orders(graphene::chain::asset_object asset) {
        if (borrow_url.empty()) return;
        
        const auto& _db = database();
        
        const auto& call_index = _db.get_index_type<call_order_index>().indices().get<by_price>();
        
        price index_price = price::min(asset.bitasset_data(_db).options.short_backing_asset, asset.get_id());
        auto itr_min = call_index.lower_bound(index_price.min());
        auto itr_max = call_index.lower_bound(index_price.max());
        
        ofstream out(write_to_file_call_order_name,ios::app);
        
        while( itr_min != itr_max )
        {
            vector<mutable_variant_object> vector;
            
            for (int i = 0; i < max_count && (itr_min != itr_max); i ++) {
                call_order_object call = *itr_min;
                
                const auto collateral = call.get_collateral();
                const auto &collateral_asset = collateral.asset_id(_db);
                const auto debt = call.get_debt();
                const auto &debt_asset = debt.asset_id(_db);
                const auto &borrower = call.borrower(_db);
                
                mutable_variant_object variant("name",borrower.name);
                
                variant.set("collateral", collateral_asset.amount_to_string(collateral));
                
                variant.set("debt", debt_asset.amount_to_string(debt));
                
                vector.push_back(variant);
                
                ++itr_min;
            }
            
            mutable_variant_object variant("list",vector);
            
            variant.set("symbol", asset.symbol);
            
            string json = fc::json::to_string(variant);
            
            bool result = send_json_string(borrow_url,json);
            
            if (!result) out<<json<<endl;
            
            if (variant.size() < max_count) break;
        }
        
        out.close();
        
        
    }
    
    static size_t cb(char *d, size_t n, size_t l, void *p)
    {
        (void)d;
        (void)p;
        return n*l;
    }
    
    bool upload_balances::send_json_string(std::string url,std::string json_string) {
        int http_code = 0;
        
        try
        {
            dlog("Prepare send data:${json}",("json",json_string));
            CURL *curl = NULL;
            CURLcode res;
            
            curl = curl_easy_init();
            
            if (NULL != curl)
            {
                
                
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
                // 设置超时时间为1秒
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, post_wait_time);
                
                // First set the URL that is about to receive our POST.
                // This URL can just as well be a
                // https:// URL if that is what should receive the data.
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                //curl_easy_setopt(pCurl, CURLOPT_URL, "http://192.168.0.2/posttest.cgi");
                
                // 设置http发送的内容类型为JSON
                curl_slist *plist = curl_slist_append(NULL,
                                                      "Content-Type:application/json;charset=UTF-8");
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
                
                // 设置要POST的JSON数据
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string.c_str());
                
                // Perform the request, res will get the return code
                res = curl_easy_perform(curl);
                curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
                // always cleanup
                curl_easy_cleanup(curl);
            }
            curl_global_cleanup();
        }catch (std::exception &ex)
        {
            elog("curl exception ${error}.\n",("error",ex.what()));
        }
            
        return http_code == 200;
//        // In windows, this will init the winsock stuff
//        curl_global_init(CURL_GLOBAL_ALL);
//        struct curl_slist *headers = NULL;
//        curl_slist_append(headers, "Content-Type: application/json");
    }
    
    vector<variant> upload_balances::get_account_balances(account_id_type id) {
        vector<variant> result;
        const account_balance_index& balance_index = database().get_index_type<account_balance_index>();
        auto range = balance_index.indices().get<by_account_asset>().equal_range(boost::make_tuple(id));
        for (const account_balance_object& balance : boost::make_iterator_range(range.first, range.second)) {
            asset assets(balance.get_balance());
            
            mutable_variant_object variants("asset_id",assets.asset_id);
            
            variants.set("asset_amount", assets.amount.value);
            
            asset_object obj = assets.asset_id(database());
            
            variants.set("asset_symbol", obj.symbol);
            
            variants.set("asset_precision", obj.precision);
            
            result.push_back(variants);
        }
        
        return result;
    }
} }
