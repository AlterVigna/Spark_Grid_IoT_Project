package iot.unipi.it.JSON;

import java.util.ArrayList;
import java.util.List;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * This class represent the generic object containing several measurements
 * arrived from a sensor accordingly to SenML, to be stored on DB.
 * 
 * @author d.vigna
 */
public class SenMLObject {

	private String baseName;
	private int baseTime;
	private String baseUnit;
	private int version;

	private List<SenMLMeasurment> measurments = new ArrayList<SenMLMeasurment>();

	public SenMLObject(String json) {

		try {
			JSONObject jsonObject = new JSONObject(json);

			try {
				this.baseName = jsonObject.getString("bn");
			} catch (JSONException jex) {

			}

			try {
				this.baseTime = jsonObject.getInt("bt");
			} catch (JSONException jex) {

			}

			try {
				this.baseUnit = jsonObject.getString("bu");
			} catch (JSONException jex) {

			}

			try {
				this.version = jsonObject.getInt("ver");
			} catch (JSONException jex) {

			}

			JSONArray jsonArray = jsonObject.getJSONArray("e");

			for (int i = 0; i < jsonArray.length(); i++) {

				JSONObject jsonMeasurment = jsonArray.getJSONObject(i);

				SenMLMeasurment measurment = new SenMLMeasurment(jsonMeasurment.toString());
				this.measurments.add(measurment);

			}

		} catch (Exception e) {
			System.out.println("Error in decoding JSON..");
		}

	}

	public String getBaseName() {
		return baseName;
	}

	public int getBaseTime() {
		return baseTime;
	}

	public String getBaseUnit() {
		return baseUnit;
	}

	public int getVersion() {
		return version;
	}

	public List<SenMLMeasurment> getMeasurments() {
		return measurments;
	}

	@Override
	public String toString() {

		String strRet = "baseName (bn): " + this.getBaseName() + " baseTime (bt): " + this.getBaseTime()
				+ " baseUnit (bu): " + this.getBaseUnit() + " version (ver): " + this.getVersion()
				+ "\n measurments:\n";
		for (SenMLMeasurment senMLMeasurment : measurments) {
			strRet += senMLMeasurment.toString();
		}

		return strRet;

	}

}
