package iot.unipi.it.coap;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.json.JSONObject;

/**
 * Utility class to prepare and send a generic client CoAP request.
 * 
 * @author d.vigna
 */
public class CoapRequest {

	/**
	 * This is a generic method to send a CoAP request to a remote device, acting as
	 * client and waiting an answer from the server (IoT device).
	 * 
	 * @param ipAddress The address to contact
	 * @param resource  The resource to be considered
	 * @param method    The method to interact with the resource
	 * @param payload   The optional information to pass
	 * @return
	 */
	public static CoapResponse sendCoapRequest(String ipAddress, String resource, String method, JSONObject payload) {

		CoapClient client = new CoapClient("coap://[" + ipAddress + "]/" + resource);
		CoapResponse response = null;

		switch (method) {
		case ("GET"):
			response = client.get();
			break;

		case ("POST"):
			response = client.put(payload.toString(), MediaTypeRegistry.APPLICATION_JSON);
			break;

		case ("PUT"):
			response = client.put(payload.toString(), MediaTypeRegistry.APPLICATION_JSON);
			break;

		case ("DELETE"):
			response = client.delete();
			break;

		}

		System.out.print(response.getResponseText());
		return response;
	}

}
