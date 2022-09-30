
loadfile("json.lua")
json=require("json")
local groups={{['group_name']='root',['key']='rootaimy'},{['group_name']='test',['key']='testaimy'}}
function loop_task ()
	local cnt=0
	while( need_execed() )
	do
		cnt = cnt+1
		json_val={}
		json_val['cmd']='vd_query_supported_command'
		json_val['session']={["cnt"]=cnt}
		json_val['params']={['groups']=groups}
		json_val['request']={}
		print(json.encode(json_val))
		send_message(json.encode(json_val))
		sleep_ms(1000)
		handle_task()
	end 
	log_message("quit loop")
end

function hand_message (message)
	json_result=json.decode(message)
	log_message(string.format("lua recv[%s]:%s",json_result['cmd'],message))
	if(message == "end")
	then
		disconnec_connection()
	end
end
log_message("load lua")
