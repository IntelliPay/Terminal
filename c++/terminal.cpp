


#include "terminal.h"

  namespace IntelliPayTerminal {

      inline bool INTELLIPAY_DLL init (string port, string terminal_type = "v400c-plus", int baud_rate = 9600, serial::bytesize_t byte_size = serial::eightbits, serial::stopbits_t stop_bit = serial::stopbits_one, serial::flowcontrol_t xonxoff = serial::flowcontrol_none, bool rtscts = false, int timeout = 0) {
        try {

            intellipay_terminal_type = terminal_type;
            intellipay_serial_port = port;

            // let's set up the connection to the terminal device.
            stty.setPort(intellipay_serial_port);
            stty.setBaudrate(baud_rate);
            stty.setStopbits(stop_bit);
            stty.setBytesize(byte_size);
            stty.setFlowcontrol(xonxoff);
            stty.setRTS(rtscts);
            stty.open();
            if (!stty.isOpen()) {
                return false;
            }
        } catch (const exception e) {
            return false;
        }
        return true;
    }

    inline json INTELLIPAY_DLL process(string action, float amount, json options = json({})) {
        boost::algorithm::to_upper(action);
        if (action != "SALE" && action != "REFUND" && action != "VOID") {
            throw "invalid action.";
        }

        json obj;
        obj["Operation"] = action;

        string search_fields[5] = {"Tip", "Invoice", "Clerk", "PaymentItem", "PaymentID"};

        for (int x = 0; x < search_fields->length(); x++) {
            if (options.contains(search_fields[x])) {
                obj[search_fields[x]] = options[search_fields[x]];
            }
        }

        json response;
        response["status"] = "failed";

        if (!stty.isOpen()) {
            stty.open();

            if (!stty.isOpen())
                return response;
        }

        stty.write(obj.dump());

        string read_buffer;
        stty.readline(read_buffer, 256, "\n");
        if (!read_buffer.length()) {
            response["message"] = "Error reading from terminal device.";
        }

        json terminal_response = json::parse(read_buffer);

        stty.close();
        if (terminal_response.contains("Response")) {
            if (terminal_response["Response"].contains("PaymentID")) {
                response["payment-id"] = terminal_response["Response"]["PaymentID"];
            }

            if (terminal_response["Response"].contains("ResponseCode") && terminal_response["Response"]["ResponseCode"] == "00") {
                response["status"] = "processed";
            }

            if (terminal_response["Response"].contains("Operation") &&
                (terminal_response["Response"]["Operation"] == "SALE" || terminal_response["Response"]["Operation"] == "REFUND") ){
                response["card-information"] = json({});
                response["card-information"]["brand"] = terminal_response["Response"]["Card"]["Brand"];
                response["card-information"]["last4"] = terminal_response["Response"]["Card"]["Last4"];
                response["card-information"]["expiration"] = terminal_response["Response"]["Card"]["ExpDate"];
            }
        }
        return response;
    }

    inline json INTELLIPAY_DLL sale(float amount, json options = json({})) {
        return process("sale", amount, options);
    }

    inline json INTELLIPAY_DLL refund(float amount, json options = json({})){
        return process("refund", amount, options);
    }
}