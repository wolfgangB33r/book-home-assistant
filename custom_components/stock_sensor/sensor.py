'''
Custom home assistant stock sensor that receives the current stock values for a given stock symbol from www.alphavantage.co
author: Wolfgang Beer

'''

from homeassistant.helpers.entity import Entity
from homeassistant.const import CONF_NAME

import requests
import logging
from datetime import datetime, timedelta

_LOGGER = logging.getLogger(__name__)

SCAN_INTERVAL = timedelta(minutes=10)

__version__ = '1.0.0'

MONITORED_CONDITIONS = [
    'price',
    'changePercent'
]

def setup_platform(hass, config, add_devices, discovery_info=None):
    """Setup the sensor platform."""
    name = config.get(CONF_NAME)
    stocksymbol = config.get("stocksymbol")
    apikey = config.get("apikey")
    add_devices([StockSensor(name, stocksymbol, apikey)])




class StockSensor(Entity):
    """Representation of a Sensor."""

    def __init__(self, name, stocksymbol, apikey):
        """Initialize the sensor."""
        self._state = None
        self._name = name
        self._stocksymbol = stocksymbol
        self._apikey = apikey
        self._icon = "mdi:cash"
        self._state_attributes = {}

    @property
    def name(self):
        """Return the name of the sensor."""
        return self._name

    @property
    def icon(self):
        """Icon to use in the frontend, if any."""
        return self._icon

    @property
    def state(self):
        """Return the state of the sensor."""
        return self._state

    @property
    def device_state_attributes(self):
        """Return the state attributes of the sensor."""
        return self._state_attributes

    @property
    def unit_of_measurement(self):
        """Return the unit of measurement."""
        return "" 

    def update(self):
        """
        Fetch new state data for the sensor.
        This is the only method that should fetch new data for Home Assistant.
        https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=MSFT&
        payload example below:
        {
            "Global Quote": {
            "01. symbol": "UQA.VIE",
            "02. open": "8.2100",
            "03. high": "8.2500",
            "04. low": "8.1400",
            "05. price": "8.1950",
            "06. volume": "29528",
            "07. latest trading day": "2019-07-10",
            "08. previous close": "8.2100",
            "09. change": "-0.0150",
            "10. change percent": "-0.1827%"
            }
        }
        """ 
        url = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=" + str(self._stocksymbol) + "&apikey=" + str(self._apikey) + "&datatype=json"
        req = requests.get(url, headers={})
        
        response = req.json()
        #_LOGGER.error("" + req.text)
        
        self._state_attributes["open"] = float(response['Global Quote']['02. open'])
        self._state_attributes["high"] = float(response['Global Quote']['03. high'])
        self._state_attributes["low"] = float(response['Global Quote']['04. low'])
        self._state_attributes["price"] = float(response['Global Quote']['05. price'])
        self._state_attributes["volume"] = float(response['Global Quote']['06. volume'])
        self._state_attributes["previousclose"] = float(response['Global Quote']['08. previous close'])
        self._state_attributes["change"] = float(response['Global Quote']['09. change'])
        self._state_attributes["changepercent"] = float(response['Global Quote']['10. change percent'].replace('%',''))
        self._state = float(response['Global Quote']['05. price'])
        