local persist_default = true -- persistence switch
local persist_timeout = 60000   -- persistence timeout
local persist_across_vs = false -- persistence don't acrossing VS
local congestion_send_timeout = 1000 -- congestion timeout(ms) for sending
local congestion_read_timeout = 1000 -- congestion timeout(ms) for reading
local client_read_timeout  = 100000 -- timeout(ms) for reading client data
local client_send_timeout  = 100000 -- timeout(ms) for sending client data
local server_connect_timeout  = 100000 -- timeout(ms) for connecting RS 
local server_read_timeout  = 100000 -- timeout(ms) for reading server data
local server_send_timeout  = 100000 -- timeout(ms) for sending server data

local client_conn_opts = {
	congestion_send_timeout = congestion_send_timeout,
	congestion_read_timeout = congestion_read_timeout,
	read_timeout = client_read_timeout,
	send_timeout = client_send_timeout
}
local server_conn_opts = {
	congestion_send_timeout = congestion_send_timeout,
	congestion_read_timeout = congestion_read_timeout,
	connect_timeout = server_connect_timeout,
	read_timeout = server_read_timeout,
	send_timeout = server_send_timeout
}
------------------------------8583----------------------------------------
--NUMERIC: fixed-length
local ISOTYPE = {"BINARY", "CHAR", "NUMERIC", "LLVAR", "LLLVAR", "LLVAR_NUMERIC"}

-- parsing the following fields
local ISO8583_BIT = {
	--index, fieldname, bitmap type(B, N, A, AN, ANS, LL, LLL), data length, data length(0:string, 1:int, 2:binary)
	--["1"] = {"_bit MAP,EXTENDED ", 'B', 16, 2},
	["2"] = {"PRIMARY ACCOUNT NUMBER", ISOTYPE[4], 19, 0}, --
	["3"] = {"PROCESSING CODE", ISOTYPE[3], 6, 0}, --
	["4"] = {"Amount transaction", ISOTYPE[3], 12, 1}, --
	["5"] = {"Amount settlement", ISOTYPE[3], 12, 0},
	["6"] = {"Amount cardholder billing", ISOTYPE[3], 12, 0},
	["7"] = {"Transmission Date and time ", ISOTYPE[3], 10, 0}, --
	--["8"] = {"Amount cardholder billing fee", ISOTYPE[3], 8, 0}, --
	["9"] = {"Conversion rate settlement", ISOTYPE[3], 8, 0},
	["10"] = {"Conversion rate cardholder billing", ISOTYPE[3], 8, 0},
	["11"] = {"Systems trace audit number", ISOTYPE[3], 6, 1}, --
	["12"] = {"Time local transaction", ISOTYPE[3], 6, 0},
	["13"] = {"Date local transaction", ISOTYPE[3], 4, 0},
	["14"] = {"Date expiration", ISOTYPE[3], 4, 0},
	["15"] = {"Date settlement", ISOTYPE[3], 4, 0},
	["16"] = {"Date conversion", ISOTYPE[3], 4, 0},
	--["17"] = {"Date capture", ISOTYPE[3], 4, 0}, --
	["18"] = {"Merchant's Type", ISOTYPE[3], 4, 0}, --
	["19"] = {"Acquiring institution country code", ISOTYPE[3], 3, 0},
	["22"] = {"Point Of Service Entry Mode Code", ISOTYPE[3], 3, 0}, --
	["23"] = {"Card Sequence Number", ISOTYPE[3], 3, 0}, --
	["25"] = {"Point Of Service Condition Code", ISOTYPE[3], 2, 0}, --
	["26"] = {"Point Of Service PIN Capture Code", ISOTYPE[3], 2, 0}, --
	["28"] = {"Amount Transaction Fee", ISOTYPE[3], 9, 0}, --
	["32"] = {"Acquiring Institution Identification Code", ISOTYPE[4], 11, 0}, --
	["33"] = {"Forwarding Institution Identification Code", ISOTYPE[4], 11, 0}, --
	["35"] = {"Track 2 data", ISOTYPE[4], 37, 0}, --
	["36"] = {"Track 3 data", ISOTYPE[5], 104, 0}, --
	["37"] = {"Retrieval Reference Number", ISOTYPE[3], 12, 0}, --
	["38"] = {"Authorization Identification Response", ISOTYPE[3], 6, 0}, --
	["39"] = {"Response Code", ISOTYPE[3], 2, 0}, --
	["41"] = {"Card Acceptor Terminal Identification", ISOTYPE[3], 8, 0}, --
	["42"] = {"Card Acceptor Identification Code", ISOTYPE[3], 15, 0}, --
	["43"] = {"Card Acceptor Name/Location", ISOTYPE[3], 40, 0}, --
	["44"] = {"Additional Response Data", ISOTYPE[4], 25, 0}, --
	["45"] = {"Track 1 Data", ISOTYPE[4], 76, 0}, --
	["48"] = {"Additional Data - Private", ISOTYPE[5], 512, 0}, --
	["49"] = {"Currency Code, Transaction", ISOTYPE[3], 3, 0}, --
	["50"] = {"Currency Code, Settlement", ISOTYPE[3], 3, 0}, --
	["51"] = {"Currency Code, Cardholder Billing", ISOTYPE[3], 3, 0}, --
	["52"] = {"Personal Id Number (PIN) Data", ISOTYPE[3], 8, 0}, --
	["53"] = {"Security Related Control Information", ISOTYPE[3], 16, 0}, --
	["54"] = {"Amounts, Additional", ISOTYPE[5], 40, 0}, --
	["55"] = {"IC Card System Related Data", ISOTYPE[5], 255, 0}, --
	["56"] = {"Additional Data", ISOTYPE[5], 512, 0}, --
	["57"] = {"Additional Data Private", ISOTYPE[5], 100, 0}, --
	["59"] = {"Detail Inquiring", ISOTYPE[5], 600, 0}, --
	["60"] = {"Reserved", ISOTYPE[5], 100, 0}, --
	["61"] = {"Cardholder Authentication Information", ISOTYPE[5], 200, 0}, --
	["62"] = {"Switching Data", ISOTYPE[5], 200, 0}, --
	["63"] = {"Finacial Network Data", ISOTYPE[5], 512, 0}, --
	["70"] = {"Network Management Information Code", ISOTYPE[3], 3, 0}, --
	["90"] = {"Original Data Elements", ISOTYPE[3], 42, 0}, --
	["96"] = {"Message Security Code", ISOTYPE[3], 8, 0}, --
	["100"] = {"Receiving Institution Identification Code", ISOTYPE[4], 11, 0}, --
	["102"] = {"Account Id 1", ISOTYPE[4], 28, 0}, --
	["103"] = {"Account Id 2", ISOTYPE[4], 28, 0}, --
	["104"] = {"Additional Data", ISOTYPE[5], 512, 0}, --
	["113"] = {"Additional Data", ISOTYPE[5], 512, 0}, --
	["116"] = {"Additional Data", ISOTYPE[5], 512, 0}, --
	["117"] = {"Additional Data", ISOTYPE[5], 256, 0}, --
	["121"] = {"CUPS Reserved", ISOTYPE[5], 100, 0}, --
	["122"] = {"Acquiring Institution Reserved", ISOTYPE[5], 100, 0}, --
	["123"] = {"Issuer Institution Reserved", ISOTYPE[5], 100, 0}, --
	["128"] = {"Message Authentication Code Field", ISOTYPE[3], 8, 0}
}

local function log(f,...)
	local debug = true
	if debug then
		LOG.debug(LOG.ERROR, string.format(f,...))
	end
end

local function dec2bin(x)
	local count = 0
	local ret = {}
	while true do
		table.insert(ret, bit.band(x, 1))
		x = bit.rshift(x, 1)
		if x < 1 then
			break
		end
	end
	local ret_tmp = string.reverse(table.concat(ret, ""))
	return string.rep("0", 32-string.len(ret_tmp))..ret_tmp
end

--[[
description:insert index into table if bitmap field is 1，and retuen table
parameter:
NOTE: skip the first bit
--]]
local function bitmap_index(bitmap)
	local ret = {}
	local count = 0
	string.gsub(bitmap, "%d", function(x) count = count + 1; if x == "1" and count ~= 1 then table.insert(ret, tostring(count)) end end)
	return ret
end

--[[
description: return message length according to reject_code
parameter: reject_code, 0:one header, !0: two heders
return: message length
--]]
local function get_pkt_head_len(reject_code)
	local head_len = 46
	if reject_code ~= 0 then
		head_len = head_len + 46 
	end
	return head_len
end

--[[
description: parse 8583 message
parameter: msg pointing to message field 'MTI'
return: persistence strings
--]]
local function parse_8583(msg)
	--previous 4B in bitmap1
	local map1_l = struct.unpack(">I4", string.sub(msg, 5, 8))
	--the next 4B in bitmap1
	local map1_h = struct.unpack(">I4", string.sub(msg, 9, 12))
	--bitmap1 strings that one byte corresponds to a field
	local map_bit = dec2bin(map1_l)..dec2bin(map1_h)
	--data offset
	local field_off = 4 + 8
	if string.sub(map_bit, 1, 1) == "1" then --if first bit is 1, message contain two bitmaps and offset is 16B. otherwise bitmap is 8B
		local map2_l = struct.unpack(">I4", string.sub(msg, 13, 16)) --previous 4B in bitmap2
		local map2_h = struct.unpack(">I4", string.sub(msg, 17, 20)) --the next 4B in bitmap2
		map_bit = map_bit..dec2bin(map2_l)..dec2bin(map2_h) -- join bitmap1 to bitmap2
		field_off = field_off + 8
	end
	-- get all indexs in bitmap
	local t_bitmap = bitmap_index(map_bit)
	-- predefine some key field for matching message
	local key_fields = {["7"] = {false,""},["11"] = {false,""}, ["32"] = {false,""}, ["33"] = {false,""}, ["90"] = {false,""}}
	--fields_start point to data
	local fields_start =  field_off + 1 
	-- parse message data
	for i, ibit in ipairs(t_bitmap) do
		if ISO8583_BIT[ibit] then
			-- get content and length of field
			local field_type = ISO8583_BIT[ibit][2]
			local field_len = ISO8583_BIT[ibit][3] -- fixed-length by default
			local len_offset = 0  -- fixed-length field has no length
			if field_type == "LLVAR" then  --none BCD
				len_offset = 2
				field_len = tonumber(string.sub(msg, fields_start , fields_start + 1))
			elseif field_type == "LLLVAR" then  
				len_offset = 3
				field_len = tonumber(string.sub(msg, fields_start, fields_start + 2))
			elseif field_type == "LLVAR_NUMERIC" then --variable-length field 
				len_offset = 1
				local len_str = tostring(bit.tohex(struct.unpack(">I1", string.sub(msg, fields_start, fields_start), 2)))
				local len_t = tonumber(string.sub(len_str, 1, 1)) * 10 + tonumber(string.sub(len_str, 2, 2))
				field_len = (len_t + math.mod(len_t, 2)) / 2
			end
			-- save the predefine field content
			if key_fields[ibit] then
				key_fields[ibit][1] = true
				key_fields[ibit][2] = string.sub(msg, fields_start + len_offset , fields_start + len_offset + field_len - 1)
				log("[parse][feilds][%d][%s][%s]",ibit,ISO8583_BIT[ibit][1],key_fields[ibit][2])
			end
			-- point to next field
			fields_start = fields_start + len_offset + field_len
		else
			--log("[parse][mesage][failed][unknow field %d]",ibit)
			break
		end
	end
	if key_fields["7"][1] or key_fields["11"][1] or key_fields["32"][1] or key_fields["33"][1] or key_fields["90"][1] then
		return '[' .. key_fields["7"][2] .. '-' .. key_fields["11"][2] .. '-'  .. key_fields["32"][2] .. '-'  .. key_fields["33"][2] .. '-'  .. key_fields["90"][2] .. ']'
	else
		return nil
	end
end
----------------------------------------PERSIST----------------------------------------
local function msg_persist_add(sp_name, rs_name, key)
	if persist_default and key ~= nil then
		local across_vs = persist_across_vs and "virtual-server" or nil
		return PERSIST.add(sp_name, rs_name, "message", key, persist_timeout, across_vs)
	else
		return false
	end
end

local function msg_persist_find(sp_name, key)
	if persist_default and key ~= nil then
		local across_vs = persist_across_vs and "virtual-server" or nil
		return PERSIST.find(sp_name, "message", key, across_vs)
	else
		return nil
	end
end
----------------------------------------API----------------------------------------

-- load-balancing implemetion
local function schedule(sp_name)
	local i = 0
	local ok
	for i=1,10 do
		local rs = nil
		if sp_name then
			ok, rs = SLB.pool(sp_name)
		end
		if not ok then
			log("load-balancing failed in sp %s", sp_name)
			return nil,nil
		end
		if not rs then
			log("get rs failed in sp %s", sp_name)
			return nil,nil
		end
		local rs_conn = TCP.server:new(sp_name, rs.name, server_conn_opts)
		if rs_conn then
			log("create real server connection %s", rs.name)
			return rs_conn, rs
		end
		log("schedule sp %s at %dth time",sp_name,i)
	end
	log("schedule sp %s failed finally",sp_name)
	return nil , nil
end

local function send_msg_to_client(msg)
	local client = TCP.client:get()
	local bytes, err = client:send(msg)
	if not bytes then
		log("[sendto message %s failed: %s", err)
	else
		log("send %d message to client", bytes)
	end
	return err, bytes
end

local function send_msg(sp_name, msg, persist)
	local rs_conn = nil
	local rs_cfg = nil
	local sp_name = sp_name or nil
	rs_cfg = msg_persist_find(sp_name, persist)
	if rs_cfg then
		rs_conn = TCP.server.get(sp_name, rs_cfg.name)
		log("[persist %s %s]", rs_cfg.name, persist)
		if not rs_conn then
			rs_conn = TCP.server:new(sp_name, rs_cfg.name, server_conn_opts)
		end
	end
	if not rs_conn then
		rs_conn, rs_cfg = schedule(sp_name)
		if not rs_conn then
			return "schedule failed"
		end
		log("[schedule %s %s]", rs_cfg.name, persist)
	end
	if not rs_conn then
		log("[sendto message %s failed rs_conn=nil]", rs_cfg.name)
		return "conn invalid"
	end
	if not rs_conn:send(msg) then
		log("[sendto message %s failed rs_conn:send(msg)=nil]", rs_cfg.name)
		return "send err"
	end
	msg_persist_add(sp_name, rs_cfg.name, persist)
	LOG.log()
	return nil, rs_conn, rs_cfg.name
end

local function read_and_send_msg(sp_name, conn_obj, is_client, rs_name)
	local err
	local reader = is_client and "client" or "real server"
	local rs_conn

	log("---recv from %s", reader)
	-- 1: read header, len == 46
	local req_data, err = conn_obj:read(46)
	if req_data == nil then
		log("%s connection object closed by err %s", reader, err)
		return err
	end
	local data = req_data
	if string.sub(data, 3, 6) == nil or tonumber(string.sub(data, 3, 6)) == nil then
		log("%s get invalid length.", reader)
		return "invalid length"
	end
	local msg_len = tonumber(string.sub(data, 3, 6)) - 46
	if msg_len < 0 then
		log("%s compute invalid length.", reader)
		return "compute length"
	end
	-- 2: read message body
	log("---%s want to read body: len %d", reader, msg_len)
	req_data, err = conn_obj:read(msg_len)
	if req_data == nil then
		log("%s connection object closed by err %s in wanted reading", reader, err)
		return err
	end
	local msg = data..req_data
	if is_client then
		-- 3: parsing mblb message
		log("---parsing mblb message ")
		if string.sub(data, 42, 46) == nil or tonumber(string.sub(data, 42, 46)) == nil then
			log("%s get invalid reject_code.", reader)
			return "invalid reject_code"
		end
		local reject_code = tonumber(string.sub(data, 42, 46))
		if not reject_code then
			log("[parse message failed reject code=nil]")
		end
		local head_len = get_pkt_head_len(reject_code) --get header length
		-- jump 46B/92B header，point to MTI
		body = string.sub(msg, head_len + 1, -1)
		log("[parse message msg:%d header:%d body:%d]", msg_len, head_len, string.len(body))
		local persist = parse_8583(body)
		-- 4: persisterce or balance for selecting RS
		err, rs_conn,rs_name = send_msg(sp_name, msg, persist)
	else
		err = send_msg_to_client(msg)
	end
	return err, rs_conn, rs_name
end

when CLIENT_DATA {
	-- init client request
	local client = TCP.client:get()
	client:setopts(client_conn_opts)
	local sp_name,err = SLB.get_pool(true)
	if not sp_name then
		log("get server pool error:%s", err)
		sp_name,err = SLB.get_pool(false)
		if not sp_name then
			log("get backup server pool error:%s", err)
			return
		end
	end
	log("get sp name %s", sp_name)
	local conn = client
	local rs_conn
	local rs_name
	local is_client = true
	-- main loop
	while true do
		err,rs_conn, rs_name = read_and_send_msg(sp_name, conn, is_client, rs_name)
		if err then
			log("processing msg has err %s.", err)
			break
		end
		if is_client then
			conn = rs_conn
		else
			conn = client
		end
		is_client = not is_client
	end
}


