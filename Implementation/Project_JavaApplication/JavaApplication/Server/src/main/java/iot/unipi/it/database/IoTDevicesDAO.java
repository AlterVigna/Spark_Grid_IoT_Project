package iot.unipi.it.database;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

/**
 * This Data Access Object class is used to interact with the information in the
 * table of IoT devices on database.
 * 
 * @author d.vigna
 */
public class IoTDevicesDAO {

	/**
	 * This function retrieves the entire record of IoT Devices in table given a
	 * full name (unique) field.
	 * 
	 * @param conn     The connection to the database
	 * @param fullName The parameter to pass as full name in the query.
	 * @return The custom obj IoT Device representing the specific.
	 * @throws SQLException
	 */
	public static IoTDevice getIoTDevice(Connection conn, String fullName) throws SQLException {

		IoTDevice returnObj = null;

		int id;
		String alias;
		int type;
		String ipAddress;
		boolean status;
		float maxPower;

		String stmt = "SELECT * FROM iot_devices WHERE FULL_NAME=?";
		PreparedStatement ps = conn.prepareStatement(stmt);
		ps.setString(1, fullName);

		ResultSet res = ps.executeQuery();
		if (res.next()) {

			id = res.getInt("ID");
			ipAddress = res.getString("IP_ADDRESS");
			type = res.getInt("TYPE");
			alias = res.getString("ALIAS");
			status = res.getBoolean("STATUS");
			maxPower = res.getFloat("MAX_POWER");
			returnObj = new IoTDevice(id, fullName, type, alias, ipAddress, status, maxPower);

		}
		res.close();
		ps.close();

		return returnObj;

	}

	/**
	 * This function is used just to simplify the logic and obtain the id without
	 * insering the full name.
	 * 
	 * @param conn  The connection to the database
	 * @param alias The parameter to pass as alias in the query to identify a smart
	 *              device.
	 * @return The custom obj IoT Device representing the specific.
	 * @throws SQLException
	 */
	public static IoTDevice getIoTDeviceFromAlias(Connection conn, String alias) throws SQLException {

		IoTDevice returnObj = null;

		int id;
		String fullName;
		int type;
		String ipAddress;
		boolean status;
		float maxPower;

		String stmt = "SELECT * FROM iot_devices WHERE ALIAS=? LIMIT 1";
		PreparedStatement ps = conn.prepareStatement(stmt);
		ps.setString(1, alias);

		ResultSet res = ps.executeQuery();
		if (res.next()) {

			id = res.getInt("ID");
			ipAddress = res.getString("IP_ADDRESS");
			type = res.getInt("TYPE");
			fullName = res.getString("FULL_NAME");
			status = res.getBoolean("STATUS");
			maxPower = res.getFloat("MAX_POWER");
			returnObj = new IoTDevice(id, fullName, type, alias, ipAddress, status, maxPower);

		}
		res.close();
		ps.close();

		return returnObj;

	}

	/**
	 * This function retrieves the ip address of a device given the idDevice.
	 * 
	 * @param conn     The connection to the database
	 * @param idDevice The identificator of the record
	 * @return The string representing the ip address of the device.
	 * @throws SQLException
	 */
	public static String getIpAddress(Connection conn, int idDevice) throws SQLException {

		String ipAddress = "";
		String stmt = "SELECT IP_ADDRESS FROM iot_devices WHERE ID=?";
		PreparedStatement ps = conn.prepareStatement(stmt);
		ps.setInt(1, idDevice);

		ResultSet res = ps.executeQuery();
		if (res.next()) {
			ipAddress = res.getString("IP_ADDRESS");
		}
		res.close();
		ps.close();

		return ipAddress;
	}

	/**
	 * This function retrieves the status (enable/disabled) of a device given the
	 * idDevice.
	 * 
	 * @param conn     The connection to the database
	 * @param idDevice The identificator of the record
	 * @return The boolean representing True -> if the device is enabled, False ->
	 *         if the device is disabled
	 * @throws SQLException
	 */
	public static boolean getStatus(Connection conn, int idDevice) throws SQLException {

		boolean status = false;

		String stmt = "SELECT STATUS FROM iot_devices WHERE ID=?";
		PreparedStatement ps = conn.prepareStatement(stmt);
		ps.setInt(1, idDevice);

		ResultSet res = ps.executeQuery();
		if (res.next()) {
			status = res.getBoolean("STATUS");
		}
		res.close();
		ps.close();

		return status;
	}

	/**
	 * This function retrieves the maxPower of a building related to a given device.
	 * 
	 * @param conn     The connection to the database
	 * @param idDevice The identificator of the record
	 * @return The float representing the value of max power
	 * @throws SQLException
	 */
	public static float getMaxPower(Connection conn, int idDevice) throws SQLException {

		float maxPower = 0;

		String stmt = "SELECT MAX_POWER FROM iot_devices WHERE ID=?";
		PreparedStatement ps = conn.prepareStatement(stmt);
		ps.setInt(1, idDevice);

		ResultSet res = ps.executeQuery();
		if (res.next()) {
			maxPower = res.getFloat("MAX_POWER");
		}
		res.close();
		ps.close();

		return maxPower;
	}

	/**
	 * This function updates the status (enable/disabled) of a device given the
	 * idDevice.
	 * 
	 * @param conn     The connection to the database
	 * @param idDevice The identificator of the record
	 * @param status   The status to be updated in the record
	 * @return Integer value that represents how many record have been updated. If
	 *         >0 this means update ok.
	 * @throws SQLException
	 */
	public static int changeStatus(Connection conn, int idDevice, boolean status) throws SQLException {

		String stmt = "UPDATE iot_devices SET STATUS = ? WHERE ID = ?";

		PreparedStatement ps = conn.prepareStatement(stmt);
		ps.setBoolean(1, status);
		ps.setInt(2, idDevice);

		int res = ps.executeUpdate();

		ps.close();
		return res;

	}

	/**
	 * This function updates the maximum amount of power consumed instantaneously by
	 * a building represented by device id.
	 * 
	 * @param conn     The connection to the database
	 * @param idDevice The identificator of the record
	 * @param maxPower The maximum power allowed
	 * @return Integer value that represents how many record have been updated. If
	 *         >0 this means update ok.
	 * @throws SQLException
	 */
	public static int changeMaxPower(Connection conn, int idDevice, float maxPower) throws SQLException {

		String stmt = "UPDATE iot_devices SET MAX_POWER = ? WHERE ID = ?";

		PreparedStatement ps = conn.prepareStatement(stmt);
		ps.setFloat(1, maxPower);
		ps.setInt(2, idDevice);

		int res = ps.executeUpdate();

		ps.close();
		return res;
	}

	/**
	 * This method is used to initially populate the iot_device table in the
	 * database with the sensor read from config file.
	 * 
	 * @param fullName This identify the name of the sensor where the measurement
	 *                 arrives.
	 * @param type     This identifies the type of sensor: 1 = Smart Power Meter; 2
	 *                 Smart transformer
	 * @param alias    This is just an alias assigned to be recognized in the SQL
	 *                 table.
	 * @param ip       This represents the address location of the iot device.
	 * @param maxPower This represents the max power providable to a building linked
	 *                 to a smart power meter. This is 0 for smart transformer.
	 * @return The id of the sensor: this is used to futher inserts.
	 */
	public static int checkAndInsertIotDevice(String fullName, int type, String alias, String ip, int maxPower) {

		int idReturned = -1;
		try (Connection connection = HikariCPDataSource.getConnection()) {

			String stmt = "INSERT IGNORE INTO iot_devices (FULL_NAME,TYPE,ALIAS,IP_ADDRESS,MAX_POWER) VALUES(?,?,?,?,?)";

			// Preparing the SQL query to register the iot_node
			PreparedStatement ps = connection.prepareStatement(stmt);

			ps.setString(1, fullName);
			ps.setInt(2, type);
			ps.setString(3, alias);
			ps.setString(4, ip);
			ps.setInt(5, maxPower);

			ps.executeUpdate();
			ps.close();

			IoTDevice deviceObj = getIoTDevice(connection, fullName);
			if (deviceObj != null) {
				idReturned = deviceObj.getId();
			}

		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return idReturned;
	}

	/**
	 * Custom object to handle multiple attributes returned from a query.
	 */
	public static class IoTDevice {

		private int id;
		private String fullName;
		private int type; // 1 - Smart Power Meter; 2 - Smart Transformer
		private String alias;
		private String ipAddress;
		private boolean status;
		private float maxPower;

		public IoTDevice(int id, String fullName, int type, String alias, String ipAddress, boolean status,
				float maxPower) {
			super();
			this.id = id;
			this.fullName = fullName;
			this.type = type;
			this.alias = alias;
			this.ipAddress = ipAddress;
			this.status = status;
			this.maxPower = maxPower;
		}

		public int getId() {
			return id;
		}

		public String getFullName() {
			return fullName;
		}

		public int getType() {
			return type;
		}

		public String getAlias() {
			return alias;
		}

		public String getIpAddress() {
			return ipAddress;
		}

		public boolean isStatus() {
			return status;
		}

		public float getMaxPower() {
			return maxPower;
		}

	}

}
