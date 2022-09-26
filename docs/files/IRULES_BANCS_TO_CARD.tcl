when CLIENT_ACCEPTED {

    TCP::collect

    log local0. "CLIENT_ACCEPTED: [IP::remote_addr]:[TCP::remote_port] - [IP::local_addr]:[TCP::local_port]"

    GENERICMESSAGE::peer name [IP::remote_addr]:[TCP::remote_port]
}

when CLIENT_DATA {

    log local0. "CLIENT_DATA: [IP::remote_addr]:[TCP::remote_port] - [IP::local_addr]:[TCP::local_port]"

    set tcplen [TCP::payload length]
    set payload [TCP::payload]
    binary scan [TCP::payload] B* bit_data
    binary scan [TCP::payload] H* hex_data
    log local0. "tcplen: $tcplen, payload: $payload, bit_data: $bit_data, hex_data: $hex_data"

    set data_m  [substr $hex_data 0 52]
    set am [binary format H* $data_m]

    log local0. "data_m: $data_m, am: $am"

    GENERICMESSAGE::message create $am

    TCP::release
    TCP::collect

}

when LB_SELECTED {
    log local0. "LB_SELECTED"
}

when LB_FAILED {
    log local0. "LB_FAILED"
}

when SERVER_CONNECTED {
    GENERICMESSAGE::peer name [IP::remote_addr]:[TCP::remote_port]
    log local0. "SERVER_CONNECTED: [IP::remote_addr]:[TCP::remote_port] - [IP::local_addr]:[TCP::local_port]"
}

when SERVER_DATA {
    log local0. "SERVER_DATA: [IP::remote_addr]:[TCP::remote_port] - [IP::local_addr]:[TCP::local_port]"
}

when MR_INGRESS {
    log local0. "mr-ingress: len: [GENERICMESSAGE::message length], src: [GENERICMESSAGE::message src], dst: [GENERICMESSAGE::message dst], nexthop: [MR::message nexthop], route: [MR::message route], transport: [MR::transport]"
}

when MR_EGRESS {
    log local0. "mr-engress: len: [GENERICMESSAGE::message length], src: [GENERICMESSAGE::message src], dst: [GENERICMESSAGE::message dst], nexthop: [MR::message nexthop], route: [MR::message route], transport: [MR::transport]"
}

when MR_FAILED {
    log local0. "MR_FAILED"
}

when GENERICMESSAGE_INGRESS {
    log local0. "gm-ingress: len: [GENERICMESSAGE::message length], status: [GENERICMESSAGE::message status], isrequest: [GENERICMESSAGE::message is_request], seqnum: [GENERICMESSAGE::message request_sequence_number]"
}

when GENERICMESSAGE_EGRESS {
    GENERICMESSAGE::message no_response true
    TCP::respond [GENERICMESSAGE::message data]
    log local0. "gm-egress: len: [GENERICMESSAGE::message length], status: [GENERICMESSAGE::message status], isrequest: [GENERICMESSAGE::message is_request], seqnum: [GENERICMESSAGE::message request_sequence_number]"
}
