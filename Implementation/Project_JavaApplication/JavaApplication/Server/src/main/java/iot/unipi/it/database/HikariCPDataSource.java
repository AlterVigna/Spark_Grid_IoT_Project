package iot.unipi.it.database;

import java.io.IOException;
import java.io.InputStream;
import java.sql.Connection;
import java.sql.SQLException;
import java.util.Properties;

import javax.sql.DataSource;

import com.zaxxer.hikari.HikariConfig;
import com.zaxxer.hikari.HikariDataSource;

/**
 * This class is used to handle the connection with the database exploing the
 * mechanism of connection pooling provided by Hikari.
 * 
 * @autor d.vigna
 */
public class HikariCPDataSource {

	private static HikariDataSource dataSource;

    static {
        try {
        	
            // Load the properties from db.properties file
            Properties properties = new Properties();
            try (InputStream input = HikariCPDataSource.class.getClassLoader().getResourceAsStream("db.properties")) {
                if (input == null) {
                    System.out.println("db.properties not found");
                    throw new IOException("db.properties not found");
                }
                properties.load(input);
            }

            // Configure HikariCP with properties
            HikariConfig config = new HikariConfig();
            config.setJdbcUrl(properties.getProperty("db.url"));
            config.setUsername(properties.getProperty("db.username"));
            config.setPassword(properties.getProperty("db.password"));
            config.setDriverClassName(properties.getProperty("db.driver"));

            // HikariCP specific settings
            config.setMaximumPoolSize(Integer.parseInt(properties.getProperty("hikari.maximum-pool-size")));
            config.setConnectionTimeout(Long.parseLong(properties.getProperty("hikari.connection-timeout")));
            config.setIdleTimeout(Long.parseLong(properties.getProperty("hikari.idle-timeout")));
            config.setMaxLifetime(Long.parseLong(properties.getProperty("hikari.max-lifetime")));

            
            dataSource = new HikariDataSource(config);

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    
 	/**
 	 * Method to obtain the private datasource.
 	 * 
 	 * @return The created Data Source
 	 */
 	public static DataSource getDataSource() {
 		return dataSource;
 	}

 	/**
 	 * Method to obtain one of the connection from the pool.
 	 * 
 	 * @return A connection to the database
 	 * @throws SQLException
 	 */
 	public static Connection getConnection() throws SQLException {
 		return dataSource.getConnection();

 	}

 	/**
 	 * To close the datasource once the application is shout down
 	 */
 	public static void close() {
 		if (dataSource != null) {
 			dataSource.close();
 		}
 	}
	
	
}
