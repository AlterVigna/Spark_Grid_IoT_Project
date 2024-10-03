package iot.unipi.it;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.Arrays;
import java.util.List;
import java.util.Scanner;

import iot.unipi.it.database.HikariCPDataSource;
import iot.unipi.it.database.IoTDevicesDAO;
import iot.unipi.it.database.IoTDevicesDAO.IoTDevice;
/**
 * Main class to launch the User Application
 * @author d.vigna
 */
public class SparkGridClientApplication {

	/**
	 * Main function that starts the application.
	 * @param args
	 */
	public static void main(String args[]) {

		SmartPowerMeterRemoteFunctionalities spmHandler = new SmartPowerMeterRemoteFunctionalities();
		SmartTransformerRemoteFunctionalities stHandler = new SmartTransformerRemoteFunctionalities();

		IoTDevice iotSmartMeter = null;
		IoTDevice iotSmartTransformer = null;
		String cooja="";
		if(args.length>0 && args[0].equals("-cooja")) {
			cooja="_cooja";
			System.out.println("User application for Cooja Simulation");
		}
		try (Scanner scanner = new Scanner(System.in)) {
			List<String> optionAllowed = Arrays.asList("1", "2", "3", "4", "5", "6", "7");
			String optSelected = "";

			// First get the ids of the registered smart devices
			Connection connection = null;
			try {
				connection = HikariCPDataSource.getConnection();

				iotSmartMeter = IoTDevicesDAO.getIoTDeviceFromAlias(connection, "house_1"+cooja);
				iotSmartTransformer = IoTDevicesDAO.getIoTDeviceFromAlias(connection, "smart_transformer_1"+cooja);

				connection.close();

				if (iotSmartMeter == null || iotSmartTransformer == null) {
					System.out.println("One of the devices is not correctly registered! ");
					return;
				}

			} catch (SQLException e) {
				e.printStackTrace();
			} finally {
				try {
					connection.close();
				} catch (SQLException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}

			while (!optionAllowed.contains(optSelected)) {

				printMenu();
				optSelected = scanner.nextLine();

				if (!optionAllowed.contains(optSelected)) {
					System.out.println("Selected option non valid! ");
				}

				switch (optSelected) {

				case ("1"):
					spmHandler.checkAndChangeBuildingStatus(iotSmartMeter.getId());
					optSelected = "";
					break;

				case ("2"):
					spmHandler.changeMaxPower(iotSmartMeter.getId());
					optSelected = "";
					break;

				case ("3"):
					spmHandler.obtainRealTimeConsumption(iotSmartMeter.getId());
					optSelected = "";
					break;

				case ("4"):
					// Id iot device forced manually
					spmHandler.obtainDailyConsumptionHouse1(iotSmartMeter.getId());
					optSelected = "";
					break;

				case ("5"):
					stHandler.obtainLastSensingMeasurments(iotSmartTransformer.getId());
					optSelected = "";
					break;

				case ("6"):
					stHandler.changeTransformerSettings(iotSmartTransformer.getId());
					optSelected = "";
					break;

				case ("7"):
					System.out.println("Good bye!");
					break;

				}

			}
		}

	}

	/**
	 * Static function to print main menu
	 */
	public static void printMenu() {
		System.out.println("===================================================");
		System.out.println("|          WELCOME TO SPARK GRID - IoT            |");
		System.out.println("===================================================");
		System.out.println("|    1. Enable/Disable Energy to house_1          |");
		System.out.println("|    2. Change Max Power to house_1               |");
		System.out.println("|    3. Real time Power house_1                   |");
		System.out.println("|    4. Daily consumption house_1                 |");
		System.out.println("|    5. Current status Smart Transformer          |");
		System.out.println("|    6. Change settings Smart Transformer relay   |");
		System.out.println("|    7. Exit                                      |");
		System.out.println("===================================================");
		System.out.println("|  Please select an option by number              |");
		System.out.println("===================================================");
	}

}
