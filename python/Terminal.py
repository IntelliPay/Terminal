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
        request_json = {"Amount": amount}

        if action not in acceptable_action_types:
            raise Exception("invalid terminal action [" + action + "]")

        if action == "VOID" and "PaymentID" not in options:
            raise Exception(action + " requires a PaymentID")

        request_json["Operator"] = action

        for field in ["Tip", "Invoice", "Clerk", "PaymentItem", "PaymentID"]:
            if field in options:
                request_json[field] = options[field]

        # Send request to terminal
        self.terminal.open()

        result = {"status": "failed"}
        response = {}
        if self.terminal.is_open:
            self.terminal.flushInput()
            self.terminal.flushOutput()
            self.terminal.write(json.dumps(request_json))
            while True:
                response = json.loads(self.terminal.readline().decode('ascii'))
                break
            self.terminal.close()
        if "Response" in response and "ResponseCode" in response["Response"] and response["Response"]["ResponseCode"] == "00":
            result["status"] = "processed"  # TODO: talk to matt probably should used "approved" instead.
        return result

    def sale(self, amount, options=None):
        return self.process("SALE", amount, options)

    def refund(self, amount, options=None):
        return self.process("REFUND", amount, options)

    def void(self, amount, options=None):
        return self.process("VOID", amount, options)
