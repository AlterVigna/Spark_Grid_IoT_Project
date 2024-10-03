package iot.unipi.it.database;

import java.util.List;

import iot.unipi.it.JSON.SenMLMeasurment;

/**
 * Interface that indicate what are the methods that an observer class should
 * implement.
 * 
 * @author d.vigna
 */
public interface ObserverActions {

	/**
	 * This function is used to insert a list of generic measures on Database. This
	 * is introduced to mantain a level of abstraction.
	 * 
	 * @param idDevice
	 * @param listMeasurments
	 */
	public void insertNewMeasures(int idDevice, List<SenMLMeasurment> listMeasurments);

}
