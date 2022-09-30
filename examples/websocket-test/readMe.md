## websocketlua测试工具使用说明
proname [api_name] [host] [port] [test_lua_file_path]
依赖:json.lua lua.so
使用示例:test.lua
```
--- 加载json支持
loadfile("json.lua")
json=require("json")
local groups={{['group_name']='root',['key']='rootaimy'},{['group_name']='test',['key']='testaimy'}}
--- 此函数会被测试工具以多线程方式调用
function loop_task ()
	local cnt=0
--- 连接断开时，此标志会被c++设置成false
	while( need_execed() )
	do
		cnt = cnt+1
		json_val={}
		json_val['cmd']='vd_query_supported_command'
		json_val['session']={["cnt"]=cnt}
		json_val['params']={['groups']=groups}
		json_val['request']={}
		print(json.encode(json_val))
--- 调用c++ websocket发送接口
		send_message(json.encode(json_val))
--- 调用c++ 休眠接口
		sleep_ms(1000)
--- 处理c++ 对lua接口的调用
		handle_task()
	end
--- 调用c++ log接口 
	log_message("quit loop")
end

--- 此函数会在handle_task中调用
function hand_message (message)
	json_result=json.decode(message)
	log_message(string.format("lua recv[%s]:%s",json_result['cmd'],message))
	if(message == "end")
	then
--- 调用c++断开连接接口
		disconnec_connection()
	end
end
log_message("load lua")
```
