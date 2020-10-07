#include "../../../c++/nlohmann/json.hpp"
#include <iostream>
#include "../../../c++/terminal.h"

#pragma comment (lib, "IntelliPayTerminal.lib")

using namespace  nlohmann;
using namespace std;


int main () {
    try {
        IntelliPayTerminal::init("COM1", "v400c-plus", 9600, serial::eightbits, serial::stopbits_one, serial::flowcontrol_none, false, 0);
        json response = IntelliPayTerminal::sale(6423.00, json({}));
        if (response.contains("status") && response["status"] == "processed") {
            cout << "Thank you for paying, and have a nice day." << endl;

            if (response.contains("payment-id")) {
                // Now lets attempt to refund.
                cout << "Attempting to refund transaction..." << endl;
                json options = {{"PaymentID", response["payment-id"]}};
                json refund_response = IntelliPayTerminal::refund(6423.00, options);
                if (refund_response.contains("status") && response["status"] == "processed") {
                    cout << "Refund was successful." << endl;
                } else if (refund_response.contains("status") && response["status"] == "failed") {
                    cout << "Failed to refund transaction." << endl;
                }
            }
        } else if (response.contains("status") && response["status"] == "failed") {
            cout << "Hmm, the sale was not processed." << endl;
        }
    } catch (exception e) {
        cerr << "Exception: " << e.what() << endl;
        return 1;
    }
    return 0;
}

