from google.protobuf import duration_pb2 as _duration_pb2
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Mapping as _Mapping, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class SetTrafficPurchasedMessage(_message.Message):
    __slots__ = ["member", "physical_synchronizer_id", "serial", "total_traffic_purchased"]
    MEMBER_FIELD_NUMBER: _ClassVar[int]
    PHYSICAL_SYNCHRONIZER_ID_FIELD_NUMBER: _ClassVar[int]
    SERIAL_FIELD_NUMBER: _ClassVar[int]
    TOTAL_TRAFFIC_PURCHASED_FIELD_NUMBER: _ClassVar[int]
    member: str
    physical_synchronizer_id: str
    serial: int
    total_traffic_purchased: int
    def __init__(self, member: _Optional[str] = ..., serial: _Optional[int] = ..., total_traffic_purchased: _Optional[int] = ..., physical_synchronizer_id: _Optional[str] = ...) -> None: ...

class TrafficConsumed(_message.Message):
    __slots__ = ["base_traffic_remainder", "extra_traffic_consumed", "last_consumed_cost", "member", "sequencing_timestamp"]
    BASE_TRAFFIC_REMAINDER_FIELD_NUMBER: _ClassVar[int]
    EXTRA_TRAFFIC_CONSUMED_FIELD_NUMBER: _ClassVar[int]
    LAST_CONSUMED_COST_FIELD_NUMBER: _ClassVar[int]
    MEMBER_FIELD_NUMBER: _ClassVar[int]
    SEQUENCING_TIMESTAMP_FIELD_NUMBER: _ClassVar[int]
    base_traffic_remainder: int
    extra_traffic_consumed: int
    last_consumed_cost: int
    member: str
    sequencing_timestamp: int
    def __init__(self, member: _Optional[str] = ..., extra_traffic_consumed: _Optional[int] = ..., base_traffic_remainder: _Optional[int] = ..., last_consumed_cost: _Optional[int] = ..., sequencing_timestamp: _Optional[int] = ...) -> None: ...

class TrafficControlParameters(_message.Message):
    __slots__ = ["base_event_cost", "enforce_rate_limiting", "max_base_traffic_accumulation_duration", "max_base_traffic_amount", "read_vs_write_scaling_factor", "set_balance_request_submission_window_size"]
    BASE_EVENT_COST_FIELD_NUMBER: _ClassVar[int]
    ENFORCE_RATE_LIMITING_FIELD_NUMBER: _ClassVar[int]
    MAX_BASE_TRAFFIC_ACCUMULATION_DURATION_FIELD_NUMBER: _ClassVar[int]
    MAX_BASE_TRAFFIC_AMOUNT_FIELD_NUMBER: _ClassVar[int]
    READ_VS_WRITE_SCALING_FACTOR_FIELD_NUMBER: _ClassVar[int]
    SET_BALANCE_REQUEST_SUBMISSION_WINDOW_SIZE_FIELD_NUMBER: _ClassVar[int]
    base_event_cost: int
    enforce_rate_limiting: bool
    max_base_traffic_accumulation_duration: _duration_pb2.Duration
    max_base_traffic_amount: int
    read_vs_write_scaling_factor: int
    set_balance_request_submission_window_size: _duration_pb2.Duration
    def __init__(self, max_base_traffic_amount: _Optional[int] = ..., max_base_traffic_accumulation_duration: _Optional[_Union[_duration_pb2.Duration, _Mapping]] = ..., read_vs_write_scaling_factor: _Optional[int] = ..., set_balance_request_submission_window_size: _Optional[_Union[_duration_pb2.Duration, _Mapping]] = ..., enforce_rate_limiting: bool = ..., base_event_cost: _Optional[int] = ...) -> None: ...

class TrafficPurchased(_message.Message):
    __slots__ = ["extra_traffic_purchased", "member", "sequencing_timestamp", "serial"]
    EXTRA_TRAFFIC_PURCHASED_FIELD_NUMBER: _ClassVar[int]
    MEMBER_FIELD_NUMBER: _ClassVar[int]
    SEQUENCING_TIMESTAMP_FIELD_NUMBER: _ClassVar[int]
    SERIAL_FIELD_NUMBER: _ClassVar[int]
    extra_traffic_purchased: int
    member: str
    sequencing_timestamp: int
    serial: int
    def __init__(self, member: _Optional[str] = ..., serial: _Optional[int] = ..., extra_traffic_purchased: _Optional[int] = ..., sequencing_timestamp: _Optional[int] = ...) -> None: ...

class TrafficReceipt(_message.Message):
    __slots__ = ["base_traffic_remainder", "consumed_cost", "extra_traffic_consumed"]
    BASE_TRAFFIC_REMAINDER_FIELD_NUMBER: _ClassVar[int]
    CONSUMED_COST_FIELD_NUMBER: _ClassVar[int]
    EXTRA_TRAFFIC_CONSUMED_FIELD_NUMBER: _ClassVar[int]
    base_traffic_remainder: int
    consumed_cost: int
    extra_traffic_consumed: int
    def __init__(self, consumed_cost: _Optional[int] = ..., extra_traffic_consumed: _Optional[int] = ..., base_traffic_remainder: _Optional[int] = ...) -> None: ...

class TrafficState(_message.Message):
    __slots__ = ["base_traffic_remainder", "extra_traffic_consumed", "extra_traffic_purchased", "last_consumed_cost", "serial", "timestamp"]
    BASE_TRAFFIC_REMAINDER_FIELD_NUMBER: _ClassVar[int]
    EXTRA_TRAFFIC_CONSUMED_FIELD_NUMBER: _ClassVar[int]
    EXTRA_TRAFFIC_PURCHASED_FIELD_NUMBER: _ClassVar[int]
    LAST_CONSUMED_COST_FIELD_NUMBER: _ClassVar[int]
    SERIAL_FIELD_NUMBER: _ClassVar[int]
    TIMESTAMP_FIELD_NUMBER: _ClassVar[int]
    base_traffic_remainder: int
    extra_traffic_consumed: int
    extra_traffic_purchased: int
    last_consumed_cost: int
    serial: int
    timestamp: int
    def __init__(self, extra_traffic_purchased: _Optional[int] = ..., extra_traffic_consumed: _Optional[int] = ..., base_traffic_remainder: _Optional[int] = ..., last_consumed_cost: _Optional[int] = ..., timestamp: _Optional[int] = ..., serial: _Optional[int] = ...) -> None: ...
