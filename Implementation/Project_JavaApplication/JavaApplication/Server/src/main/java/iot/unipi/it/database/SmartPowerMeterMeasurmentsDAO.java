package iot.unipi.it.database;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.List;

import iot.unipi.it.JSON.SenMLMeasurment;

/**
 * This Data Access Object class is used to specify how to implement the
 * insertion of a new measure related to a smart power meter.
 * 
 * @author d.vigna
 */
public class SmartPowerMeterMeasurmentsDAO implements ObserverActions {

	/**
	 * This function is used to insert the single power measure on Database.
	 * 
	 * @param idDevice        This is the identificator of the smart power meter
	 *                        device in the database.
	 * @param listMeasurments contains power-> This is the value of the power
	 *                        measure by the sensor to be stored.
	 * @throws SQLException
	 */
	@Override
	public void insertNewMeasures(int idDevice, List<SenMLMeasurment> listMeasurments) {

		if (listMeasurments == null || listMeasurments.isEmpty()) {
			return;
		}

		SenMLMeasurment measurment = listMeasurments.get(0);

		// This trick was introduced on Contiki side to allow float transmission without problems.
		Integer intPower = (Integer) measurment.getValue();
		float power = (float) intPower / 100;

		Connection connection = null;
		try {

			connection = HikariCPDataSource.getConnection();

			String stmt = "INSERT INTO smart_power_meter_measurments (ID_DEVICE,POWER) VALUES(?,?)";

			// Preparing the SQL query to register the measurement of smart power meter
			PreparedStatement ps = connection.prepareStatement(stmt);
			ps.setInt(1, idDevice);
			ps.setFloat(2, power);
			ps.executeUpdate();
			ps.close();

			System.out.println("Smart Power Meter measurment insert into database value: " + power);
		} catch (SQLException e) {
			System.out.println("An error occurred during insert in DB..");
			e.printStackTrace();
		} finally {
			try {
				connection.close();
			} catch (SQLException e) {
				e.printStackTrace();
			}
		}

	}

}
