# trade.market
trade.market合约实现在中继链上进行代币交换的功能
## 功能描述
trade.market目前只提供两种功能：1.等比例兑换。2.bancor兑换
1.等比例兑换 提供代币1 和 代币2以一种固定比例的形式进行兑换
2.bancor兑换  根据bancor形式进行兑换（暂时不建议使用bancor  等以后测试通过再说）

## 操作说明

### 1. 创建交易对
功能：         void addmarket(name trade,account_name trade_maker,trade_type type,name base_chain,asset base_amount,account_name base_account,uint64_t base_weight,
               name market_chain,asset market_amount,account_name market_account,uint64_t market_weight);
示例：cleos push action market addmarket '["eos.eosc","maker",1,"eosfoce","500.0000 SYS","maker",1,"side","1000.0000 SYS","maker",2]' -p market@active maker@active
参数说明:
trade:交易对名称
trade_maker：创建交易对的账户的名称
type：交易对的类型     1.等比例兑换。    2.bancor兑换
base_chain:第一种代币所在的链
base_amount：第一个代币的金额    amount仅仅指明币种
base_account：第一个代币绑定的账户     该账户需要在创建交易对的时候先在交易对上充值一部分代币，每次从交易对取代币是打到这个账户上面
base_weight：第一个代币所占的权重          两个代币之间交换的比例是两个代币权重的比例决定的
market_chain:第二种代币所在的链
market_amount：第二个代币的金额      amount仅仅指明币种
market_account：第二个代币绑定的账户       该账户需要在创建交易对的时候先在交易对上充值一部分代币，每次从交易对取代币是打到这个账户上面
market_weight：第二个代币所占的权重        两个代币之间交换的比例是两个代币权重的比例决定的
关于权重详解：例如：base_weight=1   market_weight=2   则1个base_coin可以兑换2个market_coin

### 2. 增加抵押
功能：void addmortgage(name trade,account_name trade_maker,account_name recharge_account,asset recharge_amount,coin_type type);
示例：cleos push action relay.token trade '["eosforce","sys.bridge","side","100.0000 EOS",2,"eos.eosc;maker;1"]' -p eosforce@active
参数说明：
trade:交易对名称
trade_maker：创建交易对的账户的名称
recharge_account：付款的账户
recharge_amount：付款的金额
type：付款代币的类型            1代表base_coin 2代表market_coin
说明：新修改后增加抵押使用给合约转账模式，调用relay.token合约的trade方法         2代表 增加抵押的动作    memo--"eos.eosc;maker;1"  是用；分割的三项 第一个是交易对名称，第二个是交易对的创建者，第三个代表冲的是第一个币还是第二个币

### 3. 赎回抵押
功能：void claimmortgage(name trade,account_name market_maker,asset claim_amount,coin_type type);
示例：cleos push action market claimmortgage '["eos.eosc","maker","100.0000 SYS",1]' -p maker@active
参数说明：
trade:交易对名称
trade_maker：创建交易对的账户的名称
recharge_amount：赎回的金额
type：赎回代币的类型            1代表base_coin 2代表market_coin
赎回抵押将自动将代币打到创建交易对绑定的账户上面

### 4. 交易
功能：void exchange(name trade,account_name trade_maker,account_name account_covert,account_name account_recv,asset amount,coin_type type);
示例：cleos push action relay.token trade '["eosforce","sys.bridge","eosforce","10.0000 SYS",3,"eos.sys;biosbpa;eosforce;2"]' -p eosforce@active
参数说明：
trade:交易对名称
trade_maker：创建交易对的账户的名称
account_covert:付款的账户
account_recv：收款的账户
type：交易的类型      1代币支付base_coin 获得market_coin  2代币支付market_coin获得base_coin
该功能是唯一用户调用的功能
说明：新修改后交易使用给合约转账模式，调用relay.token合约的trade方法         3代表 交易    memo--"eos.sys;biosbpa;eosforce;2"  是用；分割的三项 第一个是交易对名称，第二个是交易对的创建者，第三个参数是收款的账户，第四个参数代表冲的是第一个币还是第二个币

### 5. 冻结交易
功能：void frozenmarket(name trade,account_name trade_maker);
示例：cleos push action market frozenmarket '["eos.eosc","maker"]' -p maker@active
参数说明：
trade:交易对名称
trade_maker：创建交易对的账户的名称
被冻结的market 不能调用exchange功能

### 6. 解冻交易
功能：void trawmarket(name trade,account_name trade_maker);
示例：cleos push action market trawmarket '["eos.eosc","maker"]' -p maker@active
参数说明：
trade:交易对名称
trade_maker：创建交易对的账户的名称

### 7. 设置固定费用
功能：void setfixedfee(name trade,account_name trade_maker,asset base,asset market);
示例：cleos push action market setfixedfee '["eos.eosc","maker","0.1000 SYS","0.2000 SYS"]' -p maker@active
参数说明：
trade:交易对名称
trade_maker：创建交易对的账户的名称
base:购买base_coin的收取的费用
market：购买market_coin时收取的费用

### 8. 设置固定比例费用
功能：void setprofee(name trade,account_name trade_maker,uint64_t base_ratio,uint64_t market_ratio);
示例：cleos push action market setprofee '["eos.eosc","maker",20,30]' -p maker@active
参数说明：
trade:交易对名称
trade_maker：创建交易对的账户的名称
base_ratio:购买base_coin的收取的费用比例  基数为10000
market：购买market_coin时收取的费用比例  基数为10000

### 9. 设置固定比例费用含最低收费
功能：void setprominfee(name trade,account_name trade_maker,uint64_t base_ratio,uint64_t market_ratio,asset base,asset market);
示例：cleos push action market setprominfee '["eos.eosc","maker",20,30,"0.1000 SYS","0.2000 SYS"]' -p maker@active
参数说明：
trade:交易对名称
trade_maker：创建交易对的账户的名称
base_ratio:购买base_coin的收取的费用比例  基数为10000
market：购买market_coin时收取的费用比例  基数为10000
base:购买base_coin的收取的最低费用
market：购买market_coin时收取的最低费用

### 10. 设置两个币种之间的兑换比例
功能：void setweight(name trade,account_name trade_maker,uint64_t base_weight,uint64_t market_weight);
示例：cleos push action market setweight '["eos.eosc","maker",1,2]' -p maker@active
参数说明：
trade:交易对名称
trade_maker：创建交易对的账户的名称
base_weight:第一个代币所占的权重          两个代币之间交换的比例是两个代币权重的比例决定的
market_weight:  第二个代币所占的权重          两个代币之间交换的比例是两个代币权重的比例决定的
