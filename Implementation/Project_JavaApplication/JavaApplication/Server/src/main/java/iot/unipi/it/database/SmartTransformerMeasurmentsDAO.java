package iot.unipi.it.database;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Iterator;
import java.util.List;

import iot.unipi.it.JSON.SenMLMeasurment;

/**
 * This Data Access Object class is used to specify how to implement the
 * insertion of a new measure related to a smart transformer.
 * 
 * @author d.vigna
 */
public class SmartTransformerMeasurmentsDAO implements ObserverActions {

	/**
	 * This function is used to insert the current and voltage measuruments of the
	 * Smart Transformer on the Database.
	 * 
	 * @param idDevice        This is the identificator of the smart power meter
	 *                        device in the database.
	 * @param listMeasurments contains (Ia,Ib,Ic,Va,Vb,Vc) and state (number) of the
	 *                        Smart Transformer.
	 * @throws SQLException
	 */
	@Override
	public void insertNewMeasures(int idDevice, List<SenMLMeasurment> listMeasurments) {

		if (listMeasurments == null || listMeasurments.isEmpty()) {
			return;
		}

		float Ia = 0, Ib = 0, Ic = 0, Va = 0, Vb = 0, Vc = 0;
		int state = -1;

		for (Iterator<SenMLMeasurment> iterator = listMeasurments.iterator(); iterator.hasNext();) {
			SenMLMeasurment senMLMeasurment = (SenMLMeasurment) iterator.next();

			if (senMLMeasurment.getName().equals("state")) {
				state = (int) senMLMeasurment.getValue();
				state=state/100;
			}

			if (senMLMeasurment.getName().equals("current_A")) {
				// This trick was introduced on Contiki side to allow float transmission.
				int intCurrentA = (int) senMLMeasurment.getValue();
				Ia = (float) intCurrentA / 100;
			}

			if (senMLMeasurment.getName().equals("current_B")) {
				int intCurrentB = (int) senMLMeasurment.getValue();
				Ib = (float) intCurrentB / 100;
			}

			if (senMLMeasurment.getName().equals("current_C")) {
				int intCurrentC = (int) senMLMeasurment.getValue();
				Ic = (float) intCurrentC / 100;
			}

			if (senMLMeasurment.getName().equals("voltage_A")) {
				int intVoltageA = (int) senMLMeasurment.getValue();
				Va = (float) intVoltageA / 100;
			}

			if (senMLMeasurment.getName().equals("voltage_B")) {
				int intVoltageB = (int) senMLMeasurment.getValue();
				Vb = (float) intVoltageB / 100;
			}

			if (senMLMeasurment.getName().equals("voltage_C")) {
				int intVoltageC = (int) senMLMeasurment.getValue();
				Vc = (float) intVoltageC / 100;
			}
		}
		Connection connection = null;
		try {

			connection=HikariCPDataSource.getConnection();

			String stmt = "INSERT INTO smart_transformer_sensor_measurments (ID_DEVICE,STATE,IA,IB,IC,VA,VB,VC) VALUES(?,?,?,?,?,?,?,?)";

			// Preparing the SQL query to register the measurment of smart power meter
			PreparedStatement ps = connection.prepareStatement(stmt);
			ps.setInt(1, idDevice);
			ps.setInt(2, state);

			ps.setFloat(3, Ia);
			ps.setFloat(4, Ib);
			ps.setFloat(5, Ic);

			ps.setFloat(6, Va);
			ps.setFloat(7, Vb);
			ps.setFloat(8, Vc);

			ps.executeUpdate();
			ps.close();
			

			System.out.println("Smart Transformer sensor measurment insert into database values: (state=" + state
					+ ", Ia=" + Ia + ", Ib=" + Ib+", Ic="+Ic+", Va="+Va+", Vb="+Vb+", Vc="+Vc+") ");

		} catch (SQLException e) {
			System.out.println("An error occurred during insert in DB..");
			e.printStackTrace();
		}
		finally {
			try {
				connection.close();
			} catch (SQLException e) {
				e.printStackTrace();
			}
		}
	}


}
