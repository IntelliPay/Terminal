#! /usr/bin/env python3

import time
import serial
import json


class Terminal:

    terminal_type = "v400c-plus"
    terminal = serial.Serial()

    def __init__(self, terminal_type='v400c-plus', baudrate=9600, bytesize=8, stopbit=1, xonxoff=0, rtscts=0, port=4, timeout=0):
        acceptable_terminal_types = ['v200c', 'v400c', 'v400c-plus']

        if terminal_type not in acceptable_terminal_types:
            raise Exception("invalid terminal type [" + terminal_type + "]")

        self.terminal_type = terminal_type
        self.terminal.baudrate = baudrate
        self.terminal.bytesize = bytesize
        self.terminal.stopbit = stopbit
        self.terminal.xonxoff = xonxoff
        self.terminal.rtscts = rtscts
        self.terminal.timeout = timeout
        self.terminal.port = port
        self.terminal.parity = serial.PARITY_NONE

    def process(self, action, amount, options=None):
        action = action.upper()
        acceptable_action_types = ["SALE", "REFUND", "VOID"]

        if action not in acceptable_action_types:
            raise Exception("invalid terminal action [" + action + "]")

        if action == "VOID" and "PaymentID" not in options:
            raise Exception(action + " requires a PaymentID")

        request_json = {}
        request_json["Operation"] = action
        request_json["Amount"] = amount

        if options is not None:
            for field in ["Tip", "Invoice", "Clerk", "PaymentItem", "PaymentID"]:
                if field in options:
                    request_json[field] = options[field]

        request_json = { "Request": request_json }

        # Send request to terminal
        self.terminal.open()
        if (self.terminal.isOpen()):
            print("Connection is open.")

        result = {"status": "failed"}
        response = {}
        if self.terminal.isOpen:
            try:
                self.terminal.flushInput()
                self.terminal.flushOutput()
                message = self.prep_message(json.dumps(request_json))
                self.terminal.write(bytes(message, 'ascii'))
                while True:
                    data = self.terminal.readline()
                    if len(data) > 0:
                        print("Data: {}".format(data))
                        response = json.loads(data.decode('ascii')[1:-2])
                        break
                    time.sleep(2)
            except Exception as ex:
                print("*E* {}".format(e.message))
            self.terminal.close()
        if "Response" in response:
            response = response["Response"]
            if "PaymentID" in response:
                result["payment-id"] = response["PaymentID"]
            if "ResponseCode" in response and response["ResponseCode"] == "00":
                result["status"] = "approved" 
            elif "ResponseCode" in response and response["ResponseCode"] == "99":
                result["message"] = response["ResponseMsg"]
            if "Operation" in response:
                if response["Operation"] == "SALE" or response["Operation"] == "REFUND":
                    result["card-information"] = dict()
                    result["card-information"]["brand"] = response["Card"]["Brand"]
                    result["card-information"]["last4"] = response["Card"]["Last4"]
                    result["card-information"]["expiration"] = response["Card"]["ExpDate"]
        return result

    def prep_message(self, message):
        message =  message + str(chr(3)) 
        lrc = ord(message[0])
        for i in range(1, len(message)):
            lrc ^= ord(message[i])
        prep = str(chr(2)) + message + str(chr(lrc))
        return prep

    def sale(self, amount, options=None):
        return self.process("SALE", amount, options)

    def refund(self, amount, options=None):
        return self.process("REFUND", amount, options)

    def void(self, amount, options=None):
        return self.process("VOID", amount, options)

