#include "terminal.h"


extern bool init(string terminal_type = "v400c-plus", int baud_rate = 9600, int byte_size = 8, bool stop_bit = true, bool xonxoff = false, bool rtscts = false, string port = "", int timeout = 0) {
    try {
        intellipay_terminal_type = terminal_type;
        intellipay_serial_port = port;

        // let's set up the connection to the terminal device.
        intellipay_port = open(intellipay_serial_port.c_str(), O_RDWR);
        if (tcgetattr(intellipay_port, &stty) != 0) {
            cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << endl;
        }

        // lets set up the stty

        stty.c_cflag &= ~PARENB; // Clear parity bit
        stty.c_cflag &= (stop_bit) ? CSTOPB : ~CSTOPB; // Clear stop field
        stty.c_cflag |= CS8; // 8 bits per byte  -- TODO: need to allow users to change this parameter.
        stty.c_cflag &= (rtscts) ? CRTSCTS : ~CRTSCTS; // Disable RTS/CTS hardware flow control
        stty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

        stty.c_lflag &= ~ICANON;
        stty.c_lflag &= ~ECHO; // Disable echo
        stty.c_lflag &= ~ECHOE; // Disable erasure
        stty.c_lflag &= ~ECHONL; // Disable new-line echo
        stty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
        stty.c_iflag &= (xonxoff) ? (IXON | IXOFF | IXANY) : ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
        stty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
                          ICRNL); // Disable any special handling of received bytes

        stty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
        stty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
        // stty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
        // stty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

        stty.c_cc[VTIME] = timeout;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
        stty.c_cc[VMIN] = 0;

        // Set in/out baud rate to be 9600
        cfsetispeed(&stty, (baud_rate == 9600) ? B9600 : (baud_rate == 115200) ? B115200 : B19200);
        cfsetospeed(&stty, (baud_rate == 9600) ? B9600 : (baud_rate == 115200) ? B115200 : B19200);
    } catch (const exception e) {
        return false;
    }
    return true;
}

extern json process(string action, float amount, json options = json({})) {
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

    // Let's save and validate
    if (tcsetattr(intellipay_port, TCSANOW, &stty) != 0) {
        cout << "Error " << errno << " from tcsetattr: " << strerror(errno) << endl;
    }

    const char* str = obj.dump().c_str();
    write(intellipay_port, str, strlen(str));

    char read_buffer[256];
    memset(&read_buffer, '\0', sizeof(read_buffer));
    int number_of_bytes = read(intellipay_port, &read_buffer, sizeof(read_buffer));
    if (number_of_bytes < 0) {
        response["message"] = "Error reading from terminal device.";
    }

    string terminal_raw_response = string(read_buffer);
    json terminal_response = json::parse(terminal_raw_response);

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

extern json sale(float amount, json options = json({})) {
    return process("sale", amount, options);
}

extern json refund(float amount, json options = json({})){
    return process("refund", amount, options);
}

extern void close() {
    close(intellipay_port);
}