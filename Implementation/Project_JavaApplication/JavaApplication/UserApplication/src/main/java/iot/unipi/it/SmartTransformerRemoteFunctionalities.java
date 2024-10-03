package iot.unipi.it;

import java.sql.Connection;
import java.sql.SQLException;
import java.time.format.DateTimeFormatter;
import java.util.Scanner;

import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.CoAP.ResponseCode;
import org.json.JSONObject;

import iot.unipi.it.coap.CoapRequest;
import iot.unipi.it.database.HikariCPDataSource;
import iot.unipi.it.database.IoTDevicesDAO;
import iot.unipi.it.database.SmartTransformerDAO;
import iot.unipi.it.database.SmartTransformerDAO.TransformerMeasurement;

/**
 * This class implements some of the functionalities that can be done exploiting
 * the measurements coming from the smart transformer sensor and some of the
 * functions that can be imparted to the relay actuator.
 * 
 * @author d.vigna
 */
public class SmartTransformerRemoteFunctionalities {

	/**
	 * This functionality allows the user to see the last measurement received and
	 * save on the DB of the smart transformer.
	 * 
	 * @param idDevice The id of the device associated with the smart transformer.
	 */
	@SuppressWarnings("resource")
	public void obtainLastSensingMeasurments(int idDevice) {
		Connection connection = null;
		try {
			connection = HikariCPDataSource.getConnection();

			TransformerMeasurement stMeasure = SmartTransformerDAO.obtainLastMeasurments(connection, idDevice);

			if (stMeasure != null) {
				printTransformerLastMeasure(stMeasure);
			}
			connection.close();

			System.out.print("Press any key ");
			new Scanner(System.in).nextLine();

		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			try {
				connection.close();
			} catch (SQLException e) {
				e.printStackTrace();
			}
		}
	}

	/**
	 * This method is used to change remotely the settings of the Smart Transformer
	 * in order to balance the load manually. The idea is to specify to the
	 * (relay-actuator) the values (variations) that should be achieved by the
	 * sensing activity (Ia,Ib,Ic,Va,Vb,Vc).
	 * 
	 * @param idDevice
	 */
	@SuppressWarnings("resource")
	public void changeTransformerSettings(int idDevice) {

		Connection connection = null;
		@SuppressWarnings("resource")
		Scanner scanner = new Scanner(System.in);
		try {
			connection = HikariCPDataSource.getConnection();

			float Ia = 0, Ib = 0, Ic = 0, Va = 0, Vb = 0, Vc = 0;
			String dataRead = "";

			// Read the input of each variation.
			System.out.print("Insert the variation for Ia (MA) : ");
			dataRead = scanner.nextLine();

			try {
				Ia = Float.parseFloat(dataRead);
			} catch (Exception ex) {
			}

			System.out.print("Insert the variation for Ib (MA) : ");
			dataRead = scanner.nextLine();

			try {
				Ib = Float.parseFloat(dataRead);
			} catch (Exception ex) {
			}

			System.out.print("Insert the variation for Ic (MA) : ");
			dataRead = scanner.nextLine();

			try {
				Ic = Float.parseFloat(dataRead);
			} catch (Exception ex) {
			}

			System.out.print("Insert the variation for Va (MV) : ");
			dataRead = scanner.nextLine();

			try {
				Va = Float.parseFloat(dataRead);
			} catch (Exception ex) {
			}

			System.out.print("Insert the variation for Vb (MV) : ");
			dataRead = scanner.nextLine();

			try {
				Vb = Float.parseFloat(dataRead);
			} catch (Exception ex) {
			}

			System.out.print("Insert the variation for Vc (MV) : ");
			dataRead = scanner.nextLine();

			try {
				Vc = Float.parseFloat(dataRead);
			} catch (Exception ex) {
			}

			JSONObject jsonObj = new JSONObject();
			jsonObj.put("ia", Ia);
			jsonObj.put("ib", Ib);
			jsonObj.put("ic", Ic);
			jsonObj.put("va", Va);
			jsonObj.put("vb", Vb);
			jsonObj.put("vc", Vc);

			String ipAddress = IoTDevicesDAO.getIpAddress(connection, idDevice);

			// Send the request to the IoT device.
			CoapResponse response = CoapRequest.sendCoapRequest(ipAddress, "transformer_settings", "PUT", jsonObj);

			if (response != null && response.getCode().equals(ResponseCode.CHANGED)) {
				System.out.println("Smart Transformer relay updated successfully!");
			} else {
				System.out.println("Something goes wrong!");
			}

			connection.close();

			System.out.print("Press any key ");
			new Scanner(System.in).nextLine();

		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			try {
				connection.close();
			} catch (SQLException e) {
				e.printStackTrace();
			}
		}

	}

	// Utility print function

	/**
	 * Function to print in a pretty way the last measurement of the smart
	 * transformer.
	 * 
	 * @param stMeasure
	 */
	public void printTransformerLastMeasure(TransformerMeasurement stMeasure) {

		DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");

		System.out.println("\n\n");
		System.out.println("Last measurments of the Smart Transformer");

		System.out.println("---------------------------------------------------");
		System.out.println("|                   Electrical Data               |");
		System.out.println("---------------------------------------------------");
		System.out.printf("| %-12s | %-10s | %-10s | %-10s |%n", "Current (MA)", "Ia", "Ib", "Ic");
		System.out.println("---------------------------------------------------");
		System.out.printf("| %-12s | %-10.2f | %-10.2f | %-10.2f |%n", "MegaAmps", stMeasure.getIa(), stMeasure.getIb(),
				stMeasure.getIc());
		System.out.println("---------------------------------------------------");
		System.out.printf("| %-12s | %-10s | %-10s | %-10s |%n", "Voltage (MV)", "Va", "Vb", "Vc");
		System.out.println("---------------------------------------------------");
		System.out.printf("| %-12s | %-10.2f | %-10.2f | %-10.2f |%n", "MegaVolts", stMeasure.getVa(),
				stMeasure.getVb(), stMeasure.getVc());
		System.out.println("---------------------------------------------------");
		System.out.printf("| %-12s | %-38s |%n", "Timestamp", stMeasure.getTimestamp().format(formatter));
		System.out.println("---------------------------------------------------");

	}

}
