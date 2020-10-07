package com.intellipay

import com.fazecast.jSerialComm.*
import com.google.gson.*
import com.google.gson.Gson


class Terminal(_terminalType: String = "v400c-plus", _baudRate: Int = 9600, _byteSize: Int = 8, _stopBit: Int = 1, _xonXoff: Int = 0, _rtsCts: Boolean, _port: String = "/dev/ttyUSB0", _timeout: Int = 0) {
    var terminalType = _terminalType
    var baudRate = _baudRate
    var byteSize = _byteSize
    var stopBit = _stopBit
    var xonXoff = _xonXoff
    var rtsCts = _rtsCts
    var port = _port
    var timeout = _timeout

    fun process( type: String, amount: Double, options: HashMap<String,Any>? ): String {
        val terminal = SerialPort.getCommPort(this.port)
        terminal.setComPortParameters(this.baudRate, this.byteSize, this.stopBit, 0, true)
        if (this.rtsCts)
            terminal.setRTS()
        terminal.setFlowControl(this.xonXoff)
        terminal.setComPortTimeouts(SerialPort.TIMEOUT_READ_BLOCKING, this.timeout, this.timeout)

        val result = HashMap<String, Any>()
        result["status"] = "failed"

        val action = type.toUpperCase()
        if (!listOf("SALE", "REFUND", "VOID").contains(action))
            throw Exception("invalid terminal type [$action]")

        val requestMap: HashMap<String, Any?> = HashMap<String, Any?>()
        requestMap["Amount"] = amount

        requestMap["Operator"] = action;
        if (!options.isNullOrEmpty()) {
            for (key in listOf("Tip", "Invoice", "PaymentItem", "PaymentID")) {
                if (options.containsKey(key)) {
                    requestMap[key] = options[key].toString();
                }
            }
        }
        val gson = Gson()
        val requestString :String = gson.toJson(requestMap)
        val requestByteArray :ByteArray = requestString.toByteArray(Charsets.UTF_8)
        val responseByteArray :ByteArray = ByteArray(255)
        terminal.openPort()
        terminal.writeBytes(requestByteArray, requestByteArray.size.toLong())
        terminal.readBytes(responseByteArray, responseByteArray.size.toLong())
        var responseJson : JsonObject = JsonObject().getAsJsonObject(responseByteArray.toString());
        if (responseJson.has("Response")) {
            responseJson = responseJson.getAsJsonObject("Response")
            if (responseJson.has("PaymentID"))
                result["payment-id"] = responseJson.get("PaymentID").toString()
            if (responseJson.has("ResponseCode") && responseJson.get("ResponseCode").toString() == "00")
                result["status"] = "processed"
            if (responseJson.has("Operation"))
                if (responseJson.get("Operation").toString() == "SALE" || responseJson.get("Operation").toString() == "REFUND")
                    result["card-information"] = hashMapOf(
                            "brand" to responseJson.getAsJsonObject("Card").get("Brand").toString(),
                            "last4" to responseJson.getAsJsonObject("Card").get("Last4").toString(),
                            "expiration" to responseJson.getAsJsonObject("Card").get("ExpDate").toString()
                        )
        }
        terminal.closePort()
        return result.toString()
    }

    fun sale(amount: Double, options: HashMap<String,Any>?): String {
        return process("sale", amount, options )
    }

    fun refund(amount: Double, options: HashMap<String,Any>?): String {
        return process("refund", amount, options )
    }

    fun void (amount: Double, options: HashMap<String,Any>?): String {
        return process("void", amount, options )
    }
}