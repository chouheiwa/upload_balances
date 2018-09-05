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
#pragma once

#include <graphene/app/plugin.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/operation_history_object.hpp>

using namespace graphene::chain;
using namespace std;

namespace graphene { namespace upload_balances {
   
    class upload_balances : public graphene::app::plugin {
    public:
        // 插件名称
        std::string plugin_name()const override;
        
        // 插件描述信息
        std::string plugin_description()const override;
        
        // 配置参数注册
        virtual void plugin_set_program_options(
                                                boost::program_options::options_description &command_line_options,
                                                boost::program_options::options_description &config_file_options
                                                ) override;
        
        // 插件初始化
        virtual void plugin_initialize( const boost::program_options::variables_map& options ) override;
        
        // 启动插件
        virtual void plugin_startup() override;
        
        // 停止插件
        virtual void plugin_shutdown() override;
    private:
        int post_wait_time = 45;
        
        int hour;
        int minute;
        
        string balance_url;
        string borrow_url;
        
        string write_to_file_balance_name = "upload_balance";
        
        string write_to_file_call_order_name = "call_order";
        
        vector<string> upload_assets;
        
        int max_count;
        
        void start_schedule();
        
        void send_data_upload_account_balance();
        
        void send_data_upload_asset_call_orders(asset_object asset);
        
        bool send_json_string(string url,string json_string);
        
        vector<variant> get_account_balances(account_id_type id);
    };
} } //graphene::elasticsearch



