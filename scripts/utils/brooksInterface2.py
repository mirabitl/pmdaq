import time
import hpuni as hp
import struct

class abstractBrooks:
    def __init__(self,device_id=0,**kwargs):
        self.DEBUG=False
        self.buf=b""
        self.device_id=device_id
        self.l_address=None
        # Get the address
        if (device_id==0):
            cmd=hp.read_unique_identifier(0)
            self.writeCommand(cmd)
            mn=self.readAnswer()
            print(mn)
            if ("full_response" in mn):
                print(mn["full_response"])
                self.l_address= hp.calculate_long_address(mn["manufacturer_id"],mn["manufacturer_device_type"],mn["device_id"].to_bytes(3, 'big'))
                #print(self.l_address)
                print("Device found %d %d %d %x\n" % (mn["manufacturer_id"],mn["manufacturer_device_type"],mn["device_id"],int.from_bytes(self.l_address,"big")))
                self.device_id=mn["device_id"]
                self.use_device(self.device_id)
        else:
            manufacturer_id=10
            manufacturer_device_type=50
            self.l_address= hp.calculate_long_address(manufacturer_id,manufacturer_device_type,device_id.to_bytes(3, 'big'))
            #print(self.l_address)
            print("Device found %d %d %d %x\n" % (manufacturer_id,manufacturer_device_type,device_id,int.from_bytes(self.l_address,"big")))
        self.res={}

    def use_device(self,d_id,m_id=10,m_dt=50):
        self.device_id=d_id
        self.l_address= hp.calculate_long_address(m_id,m_dt,d_id.to_bytes(3, 'big'))
    
    def noConnection(self):
        return (self.l_address==None)
    
    def info(self):
        if self.noConnection():
            return
        self.read_gas_type(1)
        self.read_gas_params(1)
        self.read_primary()
        self.read_set_point()
    def identity(self):
        r={}
        if (self.noConnection()):
            return r
        self.info()
        r["device_id"]=self.device_id
        r["gas_type"]=self.res["read_gas_type"]["name"]
        r["gas_density_unit"]=self.res["read_gas_params"]["density_unit"]
        r["gas_density"]=self.res["read_gas_params"]["density"]
        r["gas_temperature_unit"]=self.res["read_gas_params"]["temperature_unit"]
        r["gas_temperature"]=self.res["read_gas_params"]["temperature"]
        r["gas_pressure_unit"]=self.res["read_gas_params"]["pressure_unit"]
        r["gas_pressure"]=self.res["read_gas_params"]["pressure"]
        r["gas_flow_unit"]=self.res["read_gas_params"]["flow_unit"]
        r["gas_flow_range"]=self.res["read_gas_params"]["flow_range"]
        return r
    def status(self):
        r={}
        if (self.noConnection()):
            return r
        self.info()
        
        r["setpoint_percent_unit"]= self.res["read_set_point"]["setpoint_percent_unit"]
        r["setpoint_percent"]= self.res["read_set_point"]["setpoint_percent"]
        r["setpoint_selected_unit"]= self.res["read_set_point"]["selected_unit"]
        r["setpoint_selected"]= self.res["read_set_point"]["setpoint_selected"]
        r["primary_unit"]=self.res["read_primary"]["primary_variable_units"]
        r["primary_variable"]=self.res["read_primary"]["primary_variable"]
        return r

    def set_flow(self,p):
        r={}
        if (self.noConnection()):
            return r
        if not "flow" in p:
            print("no flow value in ",p)
            return r
        if "device_id" in p:
            self.use_device(p["device_id"])
        self.write_set_point(p["flow"])
        
        r["setpoint_percent_unit"]= self.res["write_set_point"]["setpoint_percent_unit"]
        r["setpoint_percent"]= self.res["write_set_point"]["setpoint_percent"]
        r["setpoint_selected_unit"]= self.res["write_set_point"]["selected_unit"]
        r["setpoint_selected"]= self.res["write_set_point"]["setpoint_selected"]
        return r
    def read_primary(self):
        cmd=hp.read_primary_variable(self.l_address)
        self.writeCommand(cmd)
        rc=self.readAnswer()
        if (self.DEBUG):
            print(rc)
        self.res["read_primary"]=rc

    def read_set_point(self):
        cmd=hp.pack_command(self.l_address, command_id=235)
        self.writeCommand(cmd)
        rc=self.readAnswer()
        #self.read_setpoint=self.bparse(ra.full_response)
        if (self.DEBUG):
            print(rc)
        self.res["read_set_point"]=rc
    def write_set_point(self,percent):
        code = 57
        value = percent
        pdata = struct.pack(">Bf", code, value)
        #print(pdata)
        cmd = hp.pack_command(self.l_address, command_id=236, data=pdata)
        self.writeCommand(cmd)
        rc=self.readAnswer()
        if (self.DEBUG):
            print(rc)
        self.res["write_set_point"]=rc

    def read_gas_type(self,g_code):
        if (self.noConnection()):
            return {}
        code = g_code
        pdata = struct.pack(">B",code)
        #print(pdata)
        cmd = hp.pack_command(self.l_address, command_id=150, data=pdata)
        self.writeCommand(cmd)
        rc=self.readAnswer()
        if (self.DEBUG):
            print(rc)
        #print("READ GAS TYPE",rc)
        self.res["read_gas_type"]=rc
        
    def read_gas_params(self,g_code):
        code = g_code
        pdata = struct.pack(">B",code)
        #print(pdata)
        cmd = hp.pack_command(self.l_address, command_id=151, data=pdata)
        self.writeCommand(cmd)
        rc=self.readAnswer()
        if (self.DEBUG):
            print(rc)
        self.res["read_gas_params"]=rc

        
    def readAnswer(self):
        time.sleep(0.2)
        rep={}
        try:
            rep=self.read_response()
        except RuntimeError:
            print("Cannot read byte")
        return rep
            
        #for msg in self.unpacker:
        #    return (msg)	

    def read_response(self):
        # must work with at least two bytes to start with
        while len(self.buf) < 3:
            b=self.read_one_byte()
            if (b!=None):
                self.buf += b
            else:
                raise RuntimeError("Cannot read byte")
        # keep reading until we find a minimum preamble
        while self.buf[:3] not in [b"\xFF\xFF\x06", b"\xFF\xFF\x86"]:
            b=self.read_one_byte()
            if (b!=None):
                self.buf += b
            else:
                raise RuntimeError("Cannot read byte")

            #self.buf += self.read_one_byte()
            self.buf = self.buf[1:]
            #self._decoding_error("Head of buffer not recognized as valid preamble")
        # now the head of our buffer is the start charachter plus two preamble
        # we will read all the way through status
        if self.buf[2] & 0x80:
            l = 12
        else:
            l = 8
        while len(self.buf) < l:
            b=self.read_one_byte()
            if (b!=None):
                self.buf += b
            else:
                raise RuntimeError("Cannot read byte")

            #self.buf += self.read_one_byte()
        # now we can use the bytecount to read through the data and checksum
        bytecount = self.buf[l - 3]
        response_length = l + bytecount - 1
        while len(self.buf) < response_length:
            b=self.read_one_byte()
            if (b!=None):
                self.buf += b
            else:
                raise RuntimeError("Cannot read byte")

            #self.buf += self.read_one_byte()
        # checksum
        checksum = int.from_bytes(
            hp.calculate_checksum(self.buf[2 : response_length - 1]), "big"
        )
        if checksum != self.buf[response_length - 1]:
            print("Invalid checksum.")
            #raise StopIteration
        # parse
        response = self.buf[2:response_length]
        dict_ = self.bparse(response)
        # clear buffer
        if len(self.buf) == response_length:
            self.buf = b""
        else:
            self.buf = self.buf[response_length + 3 :]
        # return
        return dict_
    
    def bparse(self,response: bytes) -> dict():
        out= dict()
        out["full_response"] = response
        if response[0] & 0x80:  # long address
            out["address"] = int.from_bytes(response[1:6], "big")
            response = response[6:]
        else:  # short address
            out["address"] = response[1]
            response = response[2:]
        command, bytecount, response_code, device_status = struct.unpack_from(">BBBB", response)
        out["device_status"] = device_status
        out["response_code"] = response_code
        data = response[4 : 4 + bytecount]
        out["command"] = command
        out["command_name"] = f"hart_command_{command}"
        out["bytecount"] = bytecount
        out["data"] = data

        # handle error return
        if bytecount == 2:
            return out

        # universal commands
        if command in [0, 11]:
            out["command_name"] = "read_unique_identifier"
            out["manufacturer_id"] = data[1]
            out["manufacturer_device_type"] = data[2]
            out["number_response_preamble_characters"] = data[3]
            out["universal_command_revision_level"] = data[4]
            out["transmitter_specific_command_revision_level"] = data[5]
            out["software_revision_level"] = data[6]
            out["hardware_revision_level"] = data[7]
            out["device_id"] = int.from_bytes(data[9:12], "big")
        elif command in [1]:
            out["command_name"] = "read_primary_variable"
            units, variable = struct.unpack_from(">Bf", data)
            out["primary_variable_units"] = units
            out["primary_variable"] = variable
        elif command in [2]:
            out["command_name"] = "read_loop_current_and_percent"
            analog_signal, primary_variable = struct.unpack_from(">ff", data)
            out["analog_signal"] = analog_signal
            out["primary_variable"] = primary_variable
        elif command in [3]:
            out["command_name"] = "read_dynamic_variables_and_loop_current"
            (
                analog_signal,
                primary_variable_units,
                primary_variable,
                secondary_variable_units,
                secondary_variable,
            ) = struct.unpack_from(">fBfBf", data)
            out["analog_signal"] = analog_signal
            out["primary_variable_units"] = primary_variable_units
            out["primary_variable"] = primary_variable
            out["secondary_variable_units"] = secondary_variable_units
            out["secondary_variable"] = secondary_variable
        elif command in [6]:
            out["command_name"] = "write_polling_address"
            polling_address = struct.unpack_from(">B", data)[0]
            out["polling_address"] = polling_address
        elif command in [12]:
            out["command_name"] = "read_message"
            out["message"] = data[0:23]
        elif command in [13]:
            out["command_name"] = "read_tag_descriptor_date"
            out["device_tag_name"] = data[0:5]
            out["device_descriptor"] = data[6:17]
            out["date"] = data[18:20]
        elif command in [14]:
            out["command_name"] = "read_primary_variable_information"
            out["serial_no"] = data[0:2]
            sensor_limits_code, upper_limit, lower_limit, min_span = struct.unpack_from(">xxxBfff", data)
            out["sensor_limits_code"] = sensor_limits_code
            out["upper_limit"] = upper_limit
            out["lower_limit"] = lower_limit
            out["min_span"] = min_span
        elif command in [15]:
            out["command_name"] = "read_output_information"
            (
                alarm_code,
                transfer_fn_code,
                primary_variable_range_code,
                upper_range_value,
                lower_range_value,
                damping_value,
                write_protect,
                private_label,
            ) = struct.unpack_from(">BBBfffBB", data)
            out["alarm_code"] = alarm_code
            out["transfer_fn_code"] = transfer_fn_code
            out["primary_variable_range_code"] = primary_variable_range_code
            out["upper_range_value"] = upper_range_value
            out["lower_range_value"] = lower_range_value
            out["damping_value"] = damping_value
            out["write_protect"] = write_protect
            out["private_label"] = private_label
        elif command in [16]:
            out["command_name"] = "read_final_assembly_number"
            out["final_assembly_no"] = int.from_bytes(data[0:2], "big")
        elif command in [17]:
            out["command_name"] = "write_message"
            out["message"] = data[0:23]
        elif command in [18]:
            out["command_name"] = "write_tag_descriptor_date"
            out["device_tag_name"] = data[0:5]
            out["device_descriptor"] = data[6:17]
            out["date"] = data[18:20]
        elif command in [19]:
            out["command_name"] = "write_final_assembly_number"
            out["final_assembly_no"] = int.from_bytes(data[0:2], "big")

        elif command in [50]:
            out["command_name"] = "read_dynamic_variable_assignments"
            (
                primary_transmitter_variable,
                secondary_transmitter_variable,
                tertiary_transmitter_variable,
                quaternary_transmitter_variable,
            ) = struct.unpack_from(">BBBB", data)
            out["primary_transmitter_variable"] = primary_transmitter_variable
            out["secondary_transmitter_variable"] = secondary_transmitter_variable
            out["tertiary_transmitter_variable"] = tertiary_transmitter_variable  # NOT USED
            out["quaternary_transmitter_variable"] = quaternary_transmitter_variable  # NOT USED
        elif command in [59]:
            out["command_name"] = "write_number_of_response_preambles"
            n_response_preambles = struct.unpack_from(">B", data)[0]
            out["n_response_preambles"] = n_response_preambles
        elif command in [66]:
            out["command_name"] = "toggle_analog_output_mode"
            (
                analog_output_selection,
                analog_output_units_code,
                fixed_analog_output,
            ) = struct.unpack_from(">BBf", data)
            out["analog_output_selection"] = analog_output_selection
            out["analog_output_units_code"] = analog_output_units_code
            out["fixed_analog_output"] = fixed_analog_output
            
        elif command in [67]:
            out["command_name"] = "trim_analog_output_zero"
            (
                analog_output_code,
                analog_output_units_code,
                measured_analog_output
            ) = struct.unpack_from(">BBf", data)
            out["analog_output_code"] = analog_output_code
            out["analog_output_units_code"] = analog_output_units_code
            out["measured_analog_output"] = measured_analog_output
            
        elif command in [68]:
            out["command_name"] = "trim_analog_output_span"
            (
                analog_output_code,
                analog_output_units_code,
                measured_analog_output
            ) = struct.unpack_from(">BBf", data)
            
            out["analog_output_code"] = analog_output_code
            out["analog_output_units_code"] = analog_output_units_code
            out["measured_analog_output"] = measured_analog_output
        elif command in [123]:
            out["command_name"] = "select_baud_rate"
            out["baud_rate"] = int.from_bytes(data, "big")
        elif command in [235]:
            out["command_name"] = "read_set_point"
            (
                setpoint_percent_unit,
                setpoint_percent,
                selected_unit,
                setpoint_selected
            ) = struct.unpack_from(">BfBf", data)
            out["setpoint_percent_unit"] = setpoint_percent_unit
            out["setpoint_percent"] = setpoint_percent
            out["selected_unit"] = selected_unit
            out["setpoint_selected"] = setpoint_selected
        elif command in [236]:
            out["command_name"] = "write_set_point"
            (
                setpoint_percent_unit,
                setpoint_percent,
                selected_unit,
                setpoint_selected
            ) = struct.unpack_from(">BfBf", data)
            out["setpoint_percent_unit"] = setpoint_percent_unit
            out["setpoint_percent"] = setpoint_percent
            out["selected_unit"] = selected_unit
            out["setpoint_selected"] = setpoint_selected
        elif command in [150]:
            out["command_name"] = "read_gas_type"
            out["code"] = int(data[0])
            lm=12
            for i in range(1,13):
                if (data[i]==0):
                    lm=i
                    break
            if (lm!=12):
                out["name"] = data[1:lm].decode("utf8")
            else:
                out["name"] =data[1:13] 
        elif command in [151]:
            out["command_name"] = "read_gas_parameters"
            (
                code,
                density_unit,
                density,
                temp_unit,
                temperature,
                press_unit,
                pressure,
                flow_unit,
                flow_range
            ) = struct.unpack_from(">BBfBfBfBf", data)
            out["code"]= code
            out["density_unit"]=density_unit
            out["density"]=density
            out["temperature_unit"]=temp_unit
            out["temperature"]=temperature
            out["pressure_unit"]=press_unit
            out["pressure"]=pressure
            out["flow_unit"]=flow_unit
            out["flow_range"]=flow_range
        return out

   
