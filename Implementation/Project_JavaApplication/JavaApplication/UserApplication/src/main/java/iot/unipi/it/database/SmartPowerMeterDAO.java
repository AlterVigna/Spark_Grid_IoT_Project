package iot.unipi.it.database;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

/**
 * This Data Access Object class is used to interact with the information in the
 * table of measurements about the Smart Power Meter.
 * 
 * @author d.vigna
 */
public class SmartPowerMeterDAO {

	/**
	 * This method is used to retrieve from DB the general consumption statistics
	 * sent by a Smart Power Meter with specific idDevice.
	 * 
	 * @param connection The connection used to interact with db.
	 * @param idDevice   The identificator of the device
	 * @return A single custom object on the consumption info
	 * @throws SQLException
	 */
	public static GeneralInfoConsumption obtainGeneralConsumptionStatistics(Connection connection, int idDevice)
			throws SQLException {

		GeneralInfoConsumption gc = null;

		float maxPower = 0;
		float minPower = 0;
		float avgPower = 0;
		float totalConsumption = 0;

		// Reads the general information about min-max and avg power spent during the
		// day.
		String sql = "SELECT MAX(power) AS MAX_POWER, MIN(power) AS MIN_POWER, ROUND(AVG(POWER), 2) AS AVG_POWER "
				+ "FROM smart_power_meter_measurments " + "WHERE ID_DEVICE=? AND DATE(TIMESTAMP) = CURDATE()";

		PreparedStatement ps = connection.prepareStatement(sql);
		ps.setInt(1, idDevice);

		ResultSet res = ps.executeQuery();
		if (res.next()) {
			maxPower = res.getFloat("MAX_POWER");
			minPower = res.getFloat("MIN_POWER");
			avgPower = res.getFloat("AVG_POWER");
		}
		res.close();
		ps.close();

		// Here is going to compute the total power consumption like the sum of the
		// average of the power spent per hour during the day.
		String sql2 = "SELECT " + "    ROUND(SUM(hourly_avg),2) AS total_power_consumption " + "FROM (" + "    SELECT "
				+ "        AVG(POWER) AS hourly_avg " + "    FROM " + "        smart_power_meter_measurments "
				+ "    WHERE " + "        ID_DEVICE=? " + "        AND DATE(TIMESTAMP) = CURDATE() " + "    GROUP BY "
				+ "        HOUR(TIMESTAMP) " + ") AS hourly_averages ";

		PreparedStatement ps2 = connection.prepareStatement(sql2);
		ps2.setInt(1, idDevice);

		ResultSet res2 = ps2.executeQuery();
		if (res2.next()) {
			totalConsumption = res2.getFloat("total_power_consumption");
		}
		res2.close();
		ps2.close();

		gc = new GeneralInfoConsumption(minPower, maxPower, avgPower, totalConsumption);

		return gc;

	}

	/**
	 * This method is used to retrieve from DB the general consumption statistics
	 * sent by a Smart Power Meter with specific idDevice on the last hour analyzed
	 * by minute (considering the average of measurements per minute).
	 * 
	 * @param connection The connection used to interact with db.
	 * @param idDevice   The identificator of the device
	 * @return A list of custom object representing the report per minute of the
	 *         consumption on the last hour.
	 * @throws SQLException
	 */
	public static List<ReportPerHour> obtainReportPowerPerLastHour(Connection connection, int idDevice)
			throws SQLException {

		List<ReportPerHour> returnList = new ArrayList<SmartPowerMeterDAO.ReportPerHour>();

		String sql = "SELECT " + "    HOUR(TIMESTAMP) AS hour_of_day, " + "    MINUTE(TIMESTAMP) AS minute_interval, "
				+ "    ROUND(AVG(POWER),2) AS average_power " + "FROM " + "    smart_power_meter_measurments "
				+ "WHERE " + "    ID_DEVICE= ? " + "    AND DATE(TIMESTAMP) = CURDATE() "
				+ "    AND HOUR(TIMESTAMP) = (SELECT MAX(HOUR(TIMESTAMP)) "
				+ "                           FROM smart_power_meter_measurments "
				+ "                           WHERE DATE(TIMESTAMP) = CURDATE()) " + "GROUP BY "
				+ "    hour_of_day, minute_interval " + "ORDER BY " + "    hour_of_day, minute_interval ";

		PreparedStatement ps = connection.prepareStatement(sql);
		ps.setInt(1, idDevice);
		ResultSet res = ps.executeQuery();
		while (res.next()) {

			int hour = res.getInt("hour_of_day");
			int minute = res.getInt("minute_interval");
			float power = res.getFloat("average_power");

			ReportPerHour row = new ReportPerHour(hour, minute, power);
			returnList.add(row);
		}
		res.close();
		ps.close();
		return returnList;
	}

	// Utility Objects
	/**
	 * Custom object to handle multiple attributes returned from a query.
	 */
	public static class ReportPerHour {
		private int hour;
		private int minute;
		private float power;

		public ReportPerHour(int hour, int minute, float power) {
			this.hour = hour;
			this.minute = minute;
			this.power = power;
		}

		public int getHour() {
			return hour;
		}

		public int getMinute() {
			return minute;
		}

		public float getPower() {
			return power;
		}
	}

	/**
	 * Custom object to handle multiple attributes returned from a query.
	 */
	public static class GeneralInfoConsumption {
		private float minPower;
		private float maxPower;
		private float avgPower;
		private float totalConsumption;

		public GeneralInfoConsumption(float minPower, float maxPower, float avgPower, float totalConsumption) {
			this.minPower = minPower;
			this.maxPower = maxPower;
			this.avgPower = avgPower;
			this.totalConsumption = totalConsumption;
		}

		public float getMinPower() {
			return minPower;
		}

		public float getMaxPower() {
			return maxPower;
		}

		public float getAvgPower() {
			return avgPower;
		}

		public float getTotalConsumption() {
			return totalConsumption;
		}

	}

}
