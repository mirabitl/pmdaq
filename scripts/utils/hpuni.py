import math
from typing import Union


def calculate_checksum(command: Union[int, bytes]) -> bytes:
    if type(command) == int:
        command = command.to_bytes(64, "big")  # type: ignore
    lrc = 0
    for byte in command:  # type: ignore
        lrc ^= byte
    out = lrc.to_bytes(1, "big")
    return out


def calculate_long_address(manufacturer_id: int, manufacturer_device_type: int, device_id: bytes):
    out = int.from_bytes(device_id, "big")
    out |= manufacturer_device_type << 24
    out |= manufacturer_id << 32
    return out.to_bytes(5, "big")


def pack_command(address, command_id, data=None):
    if type(address) == bytes:
        address = int.from_bytes(address, "big")
    if type(command_id) == int:
        command_id = command_id.to_bytes(1, "big")
    command = b"\xFF\xFF\xFF\xFF\xFF"  # preamble
    command += b"\x82"  # start charachter
    command += (549755813888 | address).to_bytes(5, "big")
    command += command_id
    if data is None:
        command += b"\x00"  # byte count
    else:
        command += len(data).to_bytes(1, "big")  # byte count
        command += data  # data
    command += calculate_checksum(command[5:])
    return command


def pack_ascii(string: Union[str, bytes]) -> bytes:
    if type(string) == str:
        chars = [c.encode() for c in string]  # type: ignore
    else:
        chars = [c for c in string]  # type: ignore
    out = 0
    for i, c in zip(range(8), [ord(c) & 0b0011_1111 for c in chars][::-1]):
        out |= c << (i * 6)
    return out.to_bytes(math.ceil((len(string) * 6) / 8), "big")

def read_unique_identifier(address: bytes) -> bytes:
    return pack_command(address, command_id=0)


def read_primary_variable(address: bytes) -> bytes:
    return pack_command(address, command_id=1)


def read_loop_current_and_percent(address: bytes) -> bytes:
    return pack_command(address, command_id=2)


def read_dynamic_variables_and_loop_current(address: bytes) -> bytes:
    return pack_command(address, command_id=3)


def write_polling_address(address: bytes, new_polling_address: int) -> bytes:
    assert 0 <= new_polling_address <= 15
    return pack_command(address, command_id=6, data=new_polling_address.to_bytes(1, "big"))


def read_unique_identifier_associated_with_tag(tag: bytes, *, address: int = 0) -> bytes:
    return pack_command(address, command_id=11, data=tag)


def read_message(address: bytes) -> bytes:
    return pack_command(address, command_id=12)


def read_tag_descriptor_date(address: bytes) -> bytes:
    return pack_command(address, command_id=13)


def read_primary_variable_information(address: bytes) -> bytes:
    return pack_command(address, command_id=14)


def read_output_information(address: bytes) -> bytes:
    return pack_command(address, command_id=15)


def read_final_assembly_number(address: bytes) -> bytes:
    return pack_command(address, command_id=16)


def write_message(address: bytes, message: str) -> bytes:
    message = message.ljust(32)
    return pack_command(address, command_id=17, data=pack_ascii(message))


def write_tag_descriptor_date(address: bytes, tag: str, descriptor: str, date: tuple):
    data = b""
    assert len(tag) <= 8
    data += pack_ascii(tag.ljust(8))
    assert len(descriptor) <= 16
    data += pack_ascii(descriptor.ljust(16))
    day, month, year = date
    data += day.to_bytes(1, "big")
    data += month.to_bytes(1, "big")
    data += year.to_bytes(1, "big")
    return pack_command(address, command_id=18, data=data)


def write_final_assembly_number(address: bytes, number: int):
    data = number.to_bytes(3, "big")
    return pack_command(address, command_id=19, data=data)
