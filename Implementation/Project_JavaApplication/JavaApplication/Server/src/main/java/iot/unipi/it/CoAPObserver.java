package iot.unipi.it;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapObserveRelation;
import org.eclipse.californium.core.CoapResponse;

import iot.unipi.it.JSON.SenMLObject;
import iot.unipi.it.database.ObserverActions;
import iot.unipi.it.database.SmartPowerMeterMeasurmentsDAO;
import iot.unipi.it.database.SmartTransformerMeasurmentsDAO;

public class CoAPObserver {

	private CoapClient client;
	private CoapObserveRelation relation;

	private ObserverActions obsActions;

	public CoAPObserver(String ipAddress, String resource) {

		this.client = new CoapClient("coap://[" + ipAddress + "]/" + resource);

		if (resource.equals("power_obs")) {
			obsActions = new SmartPowerMeterMeasurmentsDAO();
		}

		if (resource.equals("transformer_state_obs")) {
			obsActions = new SmartTransformerMeasurmentsDAO();
		}
	}

	public void observe() {

		this.setRelation(this.client.observe(new CoapHandler() {
			@Override
			public void onLoad(CoapResponse response) {
				String content = response.getResponseText();

				System.out.println("\nArrived:" + content);

				SenMLObject senML = new SenMLObject(content);

				Integer idDevice = SparkGridServer.myCache.get(senML.getBaseName());

				if (idDevice == null) {
					System.out.println("UNKNWON DEVICE!! Measurment refused!");
					return;
				}

				if (senML.getMeasurments().isEmpty()) {
					System.out.println("No measure is present!");
					return;
				}

				obsActions.insertNewMeasures(idDevice, senML.getMeasurments());

			}

			@Override
			public void onError() {

				System.err.println("Failed observing the client : " + getClient().getURI());
				System.err.println("Cleaning and shut down the relation.");

				if (relation != null) {
					// This case is useful when the server goes down and the client can't receive the
					// message explicitly to stop observing.
					relation.reactiveCancel();
				}
				if (client != null) {
					client.shutdown();
				}
			}
		}));

	}

	public CoapClient getClient() {
		return client;
	}

	public CoapObserveRelation getRelation() {
		return relation;
	}

	public void setRelation(CoapObserveRelation relation) {
		this.relation = relation;
	}

}
