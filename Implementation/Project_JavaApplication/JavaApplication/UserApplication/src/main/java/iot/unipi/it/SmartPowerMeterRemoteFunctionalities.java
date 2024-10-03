package iot.unipi.it;

import java.sql.Connection;
import java.sql.SQLException;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Iterator;
import java.util.List;
import java.util.Scanner;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.CoAP.ResponseCode;
import org.json.JSONObject;

import iot.unipi.it.JSON.SenMLMeasurment;
import iot.unipi.it.JSON.SenMLObject;
import iot.unipi.it.coap.CoapRequest;
import iot.unipi.it.database.HikariCPDataSource;
import iot.unipi.it.database.IoTDevicesDAO;
import iot.unipi.it.database.SmartPowerMeterDAO;
import iot.unipi.it.database.SmartPowerMeterDAO.GeneralInfoConsumption;
import iot.unipi.it.database.SmartPowerMeterDAO.ReportPerHour;

/**
 * This class implements some of the functionalities that can be done exploiting
 * the measurements coming from the smart power meter sensor and some of the
 * functions that can be imparted to the actuator.
 * 
 * @author d.vigna
 */
public class SmartPowerMeterRemoteFunctionalities {

	/**
	 * The aim of this function is to realize the change the maximum power usable on
	 * remote IoT device.
	 * 
	 * @param idDevice The id of the device to be updated.
	 */
	@SuppressWarnings("resource")
	public void changeMaxPower(int idDevice) {

		Connection connection = null;

		Scanner scanner = new Scanner(System.in);

		try {
			connection = HikariCPDataSource.getConnection();

			float maxPower = IoTDevicesDAO.getMaxPower(connection, idDevice);

			printMaxPowerSmartPowerDevice("house_1", maxPower);

			// Let the user chooses if he/she wants to change the current status.
			float newMaxPower = -1;
			while (newMaxPower < 0) {
				System.out.println("Change MAX_POWER (kW):");
				String answer = scanner.nextLine();

				try {
					newMaxPower = Float.parseFloat(answer);
				} catch (Exception ex) {
					newMaxPower = -1;
				}
			}

			if (maxPower != newMaxPower) {

				String ipAddress = IoTDevicesDAO.getIpAddress(connection, idDevice);

				// Prepare the payload
				JSONObject jsonObj = new JSONObject();
				int maxPowerIntValue = (int) (newMaxPower * 1000);
				jsonObj.put("max_power", maxPowerIntValue);

				CoapResponse response = CoapRequest.sendCoapRequest(ipAddress, "max_power", "PUT", jsonObj);

				if (response != null && response.getCode().equals(ResponseCode.CHANGED)) {

					System.out.println("MAX POWER has been changed on the device successifully!");
					// Update also the database
					try {
						int ret = IoTDevicesDAO.changeMaxPower(connection, idDevice, newMaxPower);
						// All ok
						if (ret > 0) {
							System.out.println("MAX POWER update on the database successifully!");
						} else {
							throw new SQLException("Error during update of the MAX POWER on the database !");
						}
					} catch (SQLException sqlEx) {
						// Restore consistency between database and device in case of error on DB.
						sqlEx.printStackTrace();
						System.out.println("Error during update of the status on the database !");
						jsonObj.put("maxPower", maxPower);
						CoapResponse response2 = CoapRequest.sendCoapRequest(ipAddress, "status", "PUT", jsonObj);
						if (response2 != null && response2.getCode().equals(ResponseCode.CHANGED)) {
							System.out.println("MAX POWER restored on the device successifully! No inconsistency!");
						}
					}
				} else {
//					// All the negative cases ex. message lost. Do noting, consistency remains.
					System.out.println("Something goes wrong!");
				}
			} else {
				System.out.println("No modification needed! ");
			}

			connection.close();
			System.out.print("Press any key ");
			scanner.nextLine();

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
	 * The aim of this function is to realize the change of status on remote
	 * IoTDevice.
	 * 
	 * @param idDevice The id of the device on which the status has to be changed.
	 */
	@SuppressWarnings("resource")
	public void checkAndChangeBuildingStatus(int idDevice) {

		Connection connection = null;
		Scanner scanner = new Scanner(System.in);

		try {
			connection = HikariCPDataSource.getConnection();

			boolean status = IoTDevicesDAO.getStatus(connection, idDevice);

			printStatusSmartPowerDevice("house_1", status);

			// Let the user chooses if he/she wants to change the current status.
			String answer = "";
			while (!answer.toLowerCase().trim().equals("y") && !answer.toLowerCase().trim().equals("n")) {
				System.out.print("Do you want change status (y/n):");
				answer = scanner.nextLine();
			}

			if (answer.toLowerCase().trim().equals("y")) {

				String ipAddress = IoTDevicesDAO.getIpAddress(connection, idDevice);

				// Prepare the payload
				JSONObject jsonObj = new JSONObject();
				jsonObj.put("status", !status);

				CoapResponse response = CoapRequest.sendCoapRequest(ipAddress, "status", "PUT", jsonObj);

				if (response != null && response.getCode().equals(ResponseCode.CHANGED)) {

					System.out.println("Status changed on the device successifully!");
					// Update also the database
					try {
						int ret = IoTDevicesDAO.changeStatus(connection, idDevice, !status);
						// All ok
						if (ret > 0) {
							System.out.println("Status update on the database successifully!");
						} else {
							throw new SQLException("Error during update of the status on the database !");
						}
					} catch (SQLException sqlEx) {
						// Restore consistency between database and device in case of error on DB.
						sqlEx.printStackTrace();
						System.out.println("Error during update of the status on the database !");
						jsonObj.put("status", status);
						CoapResponse response2 = CoapRequest.sendCoapRequest(ipAddress, "status", "PUT", jsonObj);
						if (response2 != null && response2.getCode().equals(ResponseCode.CHANGED)) {
							System.out.println("Status restored on the device successifully! No inconsistency!");
						}
					}
				} else {
					// All the negative cases ex. message lost. Do noting, consistency remains.
					System.out.println("Something goes wrong!");
				}
			}

			connection.close();

			if (answer.toLowerCase().trim().equals("y")) {
				System.out.print("Press any key ");
				scanner.nextLine();
			}

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
	 * This function is used to read in real-time the current power measured by the
	 * sensor with idDevice. This is due to the fact that the sensor can not send
	 * any message if certain threshold is not satisfied (e.g. 1kW difference or 1
	 * minute delay) in order to flood continuously the traffic.
	 * 
	 * @param idDevice
	 */
	public void obtainRealTimeConsumption(int idDevice) {

		Connection connection = null;

		try {
			connection = HikariCPDataSource.getConnection();

			// Send a request to IoT smart power device and ask its current power measure.
			String ipAddress = IoTDevicesDAO.getIpAddress(connection, idDevice);

			CoapClient client = new CoapClient("coap://[" + ipAddress + "]/power");

			CoapResponse response = client.get();

			System.out.print(response.getResponseText());

			SenMLObject smInstantPower = new SenMLObject(response.getResponseText());

			if (smInstantPower != null && !smInstantPower.getMeasurments().isEmpty()) {
				SenMLMeasurment measure = smInstantPower.getMeasurments().get(0);
				printCurrentPowerConsumption("house_1", measure.getValueFloat());
			}

			connection.close();
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
	 * This method is used to realize option 4 of the main menu, in order to have a
	 * clear picture of the consumption of a specific building during the day.
	 * 
	 * @param idDevice - the smart device registered in the DB from which the
	 *                 measurements are registered.
	 */
	@SuppressWarnings("resource")
	public void obtainDailyConsumptionHouse1(int idDevice) {
		Connection connection = null;
		try {
			connection = HikariCPDataSource.getConnection();

			GeneralInfoConsumption gc = SmartPowerMeterDAO.obtainGeneralConsumptionStatistics(connection, idDevice);

			List<ReportPerHour> rph = SmartPowerMeterDAO.obtainReportPowerPerLastHour(connection, idDevice);

			printGeneralHouseConsumptionInfo("house_1", gc);
			printDetailedHouseConsumptionLastHour(rph);

			System.out.print("Press any key ");
			new Scanner(System.in).nextLine();

			connection.close();
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

	// Utility methods to print informations.

	/**
	 * Method to print the current power measured by a smart power sensor
	 * 
	 * @param building     The alias of the associated smart power meter
	 * @param instantPower The value of the power measured by the sensor.
	 */
	private void printCurrentPowerConsumption(String building, float instantPower) {

		LocalDateTime currentDateHour = LocalDateTime.now();
		DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");

		System.out.println("\n\n\n");
		System.out.println("Building: " + building);
		System.out.println("Time: " + currentDateHour.format(formatter));

		System.out.println("+-----------------------+");
		System.out.println("+-Current Instant Power-+");

		System.out.println("+-      " + ((instantPower > 0) ? " " : "") + String.format("%.2f", instantPower) + " W"
				+ ((Math.abs(instantPower) < 100) ? "      " : (Math.abs(instantPower) < 1000) ? "  " : " ")
				+ "    -+");
		System.out.println("+-----------------------+");

	}

	/**
	 * Method to print general house consumption information.
	 * 
	 * @param building Alias to be printed
	 * @param gc       the object containing the info obtained from a query
	 */
	private void printGeneralHouseConsumptionInfo(String building, GeneralInfoConsumption gc) {

		LocalDate currentDate = LocalDate.now();
		System.out.println("\n\n\n");
		System.out.println("Building: " + building);
		System.out.println("Date: " + currentDate);
		System.out.println("Total power consumption: " + String.format("%.2f", gc.getTotalConsumption()) + " W");
		System.out.println("Statistical daily consumption: ");
		System.out.println("+-----------------------+-----------------------+-----------------------+");
		System.out.println("|\tAVG(POWER)\t|\tMAX(POWER)\t|\tMIN(POWER)\t|");
		System.out.println("+-----------------------+-----------------------+-----------------------+");
		System.out.println(
				"|\t" + String.format("%.2f", gc.getAvgPower()) + " W\t" + ((gc.getAvgPower() < 10) ? "\t" : "") + "|\t"
						+ gc.getMaxPower() + " W\t" + ((gc.getMaxPower() < 10) ? "\t" : "") + "|\t" + gc.getMinPower()
						+ " W\t" + ((gc.getMinPower() < 10) ? "\t" : "") + "|");
		System.out.println("+-----------------------+-----------------------+-----------------------+");

	}

	/**
	 * Method to print general house consumption information per hour.
	 * 
	 * @param rph the object containing the info obtained from a query
	 */
	private void printDetailedHouseConsumptionLastHour(List<ReportPerHour> rph) {
		System.out.println("\n");
		System.out.println("Detailed Last hour:");
		System.out.println("+-----------------------+-----------------------+-----------------------+");
		System.out.println("|      hour_of_day      |     minute_interval   |     average_power     |");
		System.out.println("+-----------------------+-----------------------+-----------------------+");

		if (rph != null && !rph.isEmpty()) {

			for (Iterator<ReportPerHour> iterator = rph.iterator(); iterator.hasNext();) {
				ReportPerHour row = (ReportPerHour) iterator.next();
				System.out.println("|           " + ((row.getHour() < 10) ? "0" + row.getHour() : row.getHour())
						+ "          |          " + ((row.getMinute() < 10) ? "0" + row.getMinute() : row.getMinute())
						+ "           |       " + row.getPower() + " W       |");
			}
			System.out.println("+-----------------------+-----------------------+-----------------------+");
		}
	}

	/**
	 * This function prints the status of a specific IoT device of a building.
	 * Enabled= connected to the electrical grid, Disabled= disconnected from
	 * electrical grid
	 * 
	 * @param building The alias of the associated smart power meter
	 * @param status   The value of the status to be printed
	 */
	private void printStatusSmartPowerDevice(String building, boolean status) {

		System.out.println("\n");
		System.out.println("Building: " + building);
		System.out.println("+-----------------------+-----------------------+");
		System.out.println("|      status	        |     " + ((status) ? "Enabled" : "Disabled") + "          "
				+ ((status) ? " " : "") + "|");
		System.out.println("+-----------------------+-----------------------+");

	}

	/**
	 * This function prints the maxPower usable from a specific building related to
	 * IoT Smart Power device. Enabled= connected to the electrical grid, Disabled=
	 * disconnected from electrical grid
	 * 
	 * @param building The alias of the associated smart power meter
	 * @param status   The value of the status to be printed
	 */
	private void printMaxPowerSmartPowerDevice(String building, float maxPower) {

		System.out.println("\n");
		System.out.println("Building: " + building);
		System.out.println("+-----------------------+-----------------------+");
		System.out.println("|      Max power	|          " + maxPower + "kW        " + "|");
		System.out.println("+-----------------------+-----------------------+");

	}

}
