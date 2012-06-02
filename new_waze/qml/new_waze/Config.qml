// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    // TODO replace with a real implementation

    signal configChanged(string type, string name, string newValue)

    function getValue(type, name, defaultValue)
    {
        var value = container.data[name];
        if (typeof(value) === 'undefined')
        {
            value = defaultValue;
        }

        return value;
    }

    function setValue(type, name, value)
    {
        var oldValue = container.data[name];
        container.data[name] = value;

        if (value !== oldValue)
        {
            configChanged(type, name, value);
        }
    }

    Item {
        id: container
        property variant data: new Object()
    }
}
