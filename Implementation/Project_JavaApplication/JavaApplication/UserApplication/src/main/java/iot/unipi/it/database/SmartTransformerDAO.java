package iot.unipi.it.database;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.time.LocalDateTime;
import java.sql.Connection;

/**
 * This Data Access Object class is used to interact with the information in the
 * table of measurements about the Smart Transformer.
 * 
 * @author d.vigna
 */
public class SmartTransformerDAO {

	/**
	 * This method is used to retrieve from DB the last measure obtained from a
	 * Smart Transformer with specific device id.
	 * 
	 * @param connection The connection used to interact with db.
	 * @param idDevice   The identificator of the device
	 * @return A single custom object of the measure
	 * @throws SQLException
	 */
	public static TransformerMeasurement obtainLastMeasurments(Connection connection, int idDevice)
			throws SQLException {

		TransformerMeasurement returnObj = null;

		String sql = " select * from smart_transformer_sensor_measurments WHERE ID_DEVICE=? order by TIMESTAMP desc LIMIT 1; ";

		PreparedStatement ps = connection.prepareStatement(sql);
		ps.setInt(1, idDevice);
		ResultSet res = ps.executeQuery();

		if (res.next()) {

			int state = res.getInt("state");
			float ia = res.getFloat("ia");
			float ib = res.getFloat("ib");
			float ic = res.getFloat("ic");

			float va = res.getFloat("va");
			float vb = res.getFloat("vb");
			float vc = res.getFloat("vc");

			Timestamp timestamp = res.getTimestamp("timestamp");

			returnObj = new TransformerMeasurement(state, ia, ib, ic, va, vb, vc, timestamp);
		}

		res.close();
		ps.close();

		return returnObj;

	}

	/**
	 * Custom object to handle multiple attributes returned from a query.
	 */
	public static class TransformerMeasurement {

		private int state;
		private float Ia;
		private float Ib;
		private float Ic;
		private float Va;
		private float Vb;
		private float Vc;

		private LocalDateTime timestamp;

		public TransformerMeasurement(int status, float ia, float ib, float ic, float va, float vb, float vc,
				Timestamp timestamp) {
			super();
			this.state = status;
			Ia = ia;
			Ib = ib;
			Ic = ic;
			Va = va;
			Vb = vb;
			Vc = vc;
			this.timestamp = timestamp.toLocalDateTime();

		}

		public int getState() {
			return state;
		}

		public float getIa() {
			return Ia;
		}

		public float getIb() {
			return Ib;
		}

		public float getIc() {
			return Ic;
		}

		public float getVa() {
			return Va;
		}

		public float getVb() {
			return Vb;
		}

		public float getVc() {
			return Vc;
		}

		public LocalDateTime getTimestamp() {
			return timestamp;
		}
	}

}
