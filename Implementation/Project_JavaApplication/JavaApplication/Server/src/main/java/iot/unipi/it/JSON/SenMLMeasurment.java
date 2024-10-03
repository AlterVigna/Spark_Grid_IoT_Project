package iot.unipi.it.JSON;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * This class represent a single generic format measure arrived from a sensor accordingly to SenML, to be
 * stored on DB.
 * 
 * @author d.vigna
 */
public class SenMLMeasurment {

	private String name;
	private String unit;

	private SenmlValueType type;
	private Object value;

	private int time;

	// private float sum_value; not used in this project
	// private int updateTime; not used in this project

	public SenMLMeasurment(String json) {

		JSONObject jsonObject = new JSONObject(json);

		try {
			this.name = jsonObject.getString("n");
		} catch (JSONException jex) {

		}

		try {
			this.unit = jsonObject.getString("u");
		} catch (JSONException jex) {

		}

		Object value = null;
		try {
			this.type = SenmlValueType.SENML_TYPE_V;
			value = jsonObject.get("v");
		} catch (JSONException jex) {

		}

		try {
			this.type = SenmlValueType.SENML_TYPE_SV;
			value = jsonObject.get("sv");
		} catch (JSONException jex) {

		}

		try {
			this.type = SenmlValueType.SENML_TYPE_BV;
			value = jsonObject.get("bv");
		} catch (JSONException jex) {

		}

		this.value = value;

		try {
			this.time = jsonObject.getInt("t");
		} catch (JSONException jex) {

		}
	}

	public String getName() {
		return name;
	}

	public SenmlValueType getType() {
		return type;
	}

	public String getUnit() {
		return unit;
	}

	public Object getValue() {
		return value;
	}

	public int getTime() {
		return time;
	}

	public int getValueInt() {
		return (int) this.value / 100;
	}

	public float getValueFloat() {
		return (float) (Integer) this.value / 100;
	}

	@Override
	public String toString() {

		String strRet = " name (n): " + this.getName() + " unit (u): " + this.getUnit() + " type :" + this.getType()
				+ "value : " + this.getValue() + " time (t): " + this.getTime() + " \n";

		return strRet;
	}

}
