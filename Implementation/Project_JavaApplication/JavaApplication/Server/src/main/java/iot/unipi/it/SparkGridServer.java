package iot.unipi.it;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Properties;

import org.eclipse.californium.core.CoapServer;

import iot.unipi.it.database.IoTDevicesDAO;

/**
 * Main class to launch the Server
 * 
 * @author d.vigna
 */
public class SparkGridServer extends CoapServer {

	// Cache used to map the string used as identification of IoT device with the id
	// of the device on DB.
	public static HashMap<String, Integer> myCache = new HashMap<String, Integer>();

	public static void main(String[] args) {
		System.out.println("Starting Spark Grid Server.. \n\n");

		// Previous version: read the properties of the sensor/actuator from a file.

		/*
		 * // First read Properties properties = new Properties();
		 * 
		 * String smartPowerMeter1Alias = ""; String smartPowerMeter1Name = ""; String
		 * smartPowerMeter1Addr = ""; int smartPowerMeter1Type=1;
		 * 
		 * String smartTransformer1Alias = ""; String smartTransformer1Name = ""; String
		 * smartTransformer1Addr = ""; int smartTransformer1Type=2;
		 * 
		 * // Load the properties file from resources folder try (InputStream input =
		 * SparkGridServer.class.getClassLoader().getResourceAsStream(
		 * "iot_devices.properties")) { if (input == null) {
		 * System.out.println("Unable to find iot_devices.properties"); return; }
		 * 
		 * // Load the properties from the input stream properties.load(input);
		 * 
		 * // Read 1 by 1 the properties smartPowerMeter1Alias =
		 * properties.getProperty("smart_power_meter_1_alias"); smartPowerMeter1Name =
		 * properties.getProperty("smart_power_meter_1_name"); smartPowerMeter1Addr =
		 * properties.getProperty("smart_power_meter_1_addr"); smartPowerMeter1Type =
		 * Integer.parseInt(properties.getProperty("smart_power_meter_1_type"));
		 * 
		 * smartTransformer1Alias = properties.getProperty("smart_transformer_1_alias");
		 * smartTransformer1Name = properties.getProperty("smart_transformer_1_name");
		 * smartTransformer1Addr = properties.getProperty("smart_transformer_1_addr");
		 * smartTransformer1Type =
		 * Integer.parseInt(properties.getProperty("smart_transformer_1_type"));
		 * 
		 * int idRecord_SPM =
		 * IoTDevicesDAO.checkAndInsertIotDevice(smartPowerMeter1Name,
		 * smartPowerMeter1Type,smartPowerMeter1Alias, smartPowerMeter1Addr, 6);
		 * myCache.put(smartPowerMeter1Name, idRecord_SPM);
		 * 
		 * int idRecord_ST =
		 * IoTDevicesDAO.checkAndInsertIotDevice(smartTransformer1Name,
		 * smartTransformer1Type,smartTransformer1Alias, smartTransformer1Addr, 0);
		 * myCache.put(smartTransformer1Name, idRecord_ST);
		 * 
		 * } catch (IOException ex) { ex.printStackTrace(); }
		 * 
		 * // CoAPObserver observer = new CoAPObserver(smartPowerMeter1Addr,
		 * "power_obs"); // observer.observe();
		 * 
		 * // CoAPObserver observer2 = new CoAPObserver(smartTransformer1Addr, //
		 * "transformer_state_obs"); // observer2.observe();
		 */

		// Start the server and expose the resource for the registration.

		SparkGridServer server = new SparkGridServer();
		server.add(new DeviceRegistration("device_registration"));
		server.start();

	}

}
