package iot.unipi.it;

import java.sql.Connection;
import java.sql.SQLException;

import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.CoAP.ResponseCode;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.coap.Response;
import org.eclipse.californium.core.server.resources.CoapExchange;
import org.json.JSONObject;

import iot.unipi.it.database.HikariCPDataSource;
import iot.unipi.it.database.IoTDevicesDAO;
import iot.unipi.it.database.IoTDevicesDAO.IoTDevice;

/**
 * Class used to implement the protocol for the registration of a IoT device
 * once it starts execution. If it is a smart power meter the answer message
 * consist also in a paylod of maxPower allowed and status (enable/disable) of
 * the building.
 * 
 * @author d.vigna
 */
class DeviceRegistration extends CoapResource {

	public static int DEVICE_TYPE_SMART_POWER_METER = 1;
	public static int DEVICE_TYPE_SMART_TRANSFORMER = 2;

	public DeviceRegistration(String name) {
		super(name);
		setObservable(true);
	}

	public void handlePOST(CoapExchange exchange) {

		Response response = null;

		if (exchange.getRequestOptions().getAccept() == MediaTypeRegistry.APPLICATION_JSON) {

			String deviceFullName = "";
			String deviceAlias = " ";
			String deviceIpAddress = "";
			int deviceType = 1;

			// Get the information needed for the registration of a record on the db.
			JSONObject jsonObj = new JSONObject(exchange.getRequestText());
			deviceFullName = jsonObj.getString("full_name");
			deviceAlias = jsonObj.getString("alias");
			deviceType = jsonObj.getInt("type");
			deviceIpAddress = exchange.getSourceAddress().getCanonicalHostName();

			System.out.println("Recieved: \n(full name) ->" + deviceFullName + "\n(device type) -> "
					+ ((deviceType == 1) ? "Smart Power Meter" : "Smart Transformer") + "\n(address) -> "
					+ deviceIpAddress);
			Connection connection = null;

			try {

				connection = HikariCPDataSource.getConnection();

				int maxPower = (deviceType == DEVICE_TYPE_SMART_POWER_METER) ? 6 : 0; // By default the max power for SM
																						// is 6kW.

				// Try to insert or just retrieve the last time the device was registered, so
				// get its id.
				int idRecord = IoTDevicesDAO.checkAndInsertIotDevice(deviceFullName, deviceType, deviceAlias,
						deviceIpAddress, maxPower);
				SparkGridServer.myCache.put(deviceFullName, idRecord);

				if (deviceType == DEVICE_TYPE_SMART_POWER_METER) {

					// Answer to device Smart Power Meter its max power or if it is enable/disabled
					// (decided by energy provider).
					IoTDevice IoTDeviceObj = IoTDevicesDAO.getIoTDevice(connection, deviceFullName);
					if (IoTDeviceObj != null) {

						JSONObject jsonObjResponse = new JSONObject(exchange.getRequestText());
						jsonObjResponse.put("max_power", IoTDeviceObj.getMaxPower() * 1000);
						jsonObjResponse.put("status", IoTDeviceObj.isStatus());

						response = new Response(CoAP.ResponseCode.CREATED);
						response.setPayload(jsonObjResponse.toString());
						System.out.println("Payload sent back: " + jsonObjResponse.toString());

						CoAPObserver observer = new CoAPObserver(deviceIpAddress, "power_obs");
						observer.observe();

						System.out.println("Starting of observing power resource!");
					}
				} else {
					// The is no need for a custom answer for Smart Transformer because it is never
					// disabled from the energy provider.

					// So just start observing its resource.
					response = new Response(CoAP.ResponseCode.CREATED);

					CoAPObserver observer = new CoAPObserver(deviceIpAddress, "transformer_state_obs");
					observer.observe();
					System.out.println("Starting of observing transformer state resource!");
				}

			} catch (SQLException e) {
				System.out.println("An error occurred during operations on DB..");
				response = new Response(ResponseCode.INTERNAL_SERVER_ERROR);
				e.printStackTrace();
			} finally {
				try {
					connection.close();
				} catch (SQLException e) {
					e.printStackTrace();
				}
			}

			response.getOptions().setContentFormat(MediaTypeRegistry.APPLICATION_JSON);
		} else {
			response = new Response(ResponseCode.BAD_REQUEST);
		}

		exchange.respond(response);
	}
}