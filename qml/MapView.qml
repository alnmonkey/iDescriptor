import QtQuick 
import QtLocation 
import QtPositioning

Rectangle {
    id: mapView
    width: 800
    height: 600
    
    Plugin {
        id: osmPlugin
        name: "osm"
        
        // Simplified OSM configuration for Qt6
        PluginParameter {
            name: "osm.mapping.providersrepository.disabled"
            value: "true"
        }
        PluginParameter {
            name: "osm.mapping.providersrepository.address"
            value: "http://maps-redirect.qt.io/osm/5.6/"
        }
    }
    
    Map {
        id: map
        objectName: "map"
        plugin: osmPlugin
        anchors.fill: parent
        center: QtPositioning.coordinate(59.91, 10.75) // Oslo
        zoomLevel: 14
        property geoCoordinate startCentroid
        
        // Add a function to update center from C++
        function updateCenter(lat, lon) {
            center = QtPositioning.coordinate(lat, lon)
        }
        
        // Add marker for current location
        MapQuickItem {
            id: marker
            coordinate: map.center
            anchorPoint.x: 12
            anchorPoint.y: 12
            
            sourceItem: Rectangle {
                width: 24
                height: 24
                radius: 12
                color: "#4CAF50"
                border.color: "white"
                border.width: 2
                
                Rectangle {
                    anchors.centerIn: parent
                    width: 8
                    height: 8
                    radius: 4
                    color: "white"
                }
            }
        }
        
        // Handle click to update location
        MouseArea {
            anchors.fill: parent
            onClicked: function(mouse) {
                var coord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                map.center = coord
                marker.coordinate = coord
                console.log("Map clicked at:", coord.latitude, coord.longitude)
            }
        }
        
        PinchHandler {
            id: pinch
            target: null
            onActiveChanged: if (active) {
                map.startCentroid = map.toCoordinate(pinch.centroid.position, false)
            }
            onScaleChanged: (delta) => {
                map.zoomLevel += Math.log2(delta)
                map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position)
            }
            onRotationChanged: (delta) => {
                map.bearing -= delta
                map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position)
            }
            grabPermissions: PointerHandler.TakeOverForbidden
        }
        
        WheelHandler {
            id: wheel
            // workaround for QTBUG-87646 / QTBUG-112394 / QTBUG-112432:
            // Magic Mouse pretends to be a trackpad but doesn't work with PinchHandler
            // and we don't yet distinguish mice and trackpads on Wayland either
            acceptedDevices: Qt.platform.pluginName === "cocoa" || Qt.platform.pluginName === "wayland"
                             ? PointerDevice.Mouse | PointerDevice.TouchPad
                             : PointerDevice.Mouse
            rotationScale: 1/120
            property: "zoomLevel"
        }
        
        DragHandler {
            id: drag
            target: null
            onTranslationChanged: (delta) => map.pan(-delta.x, -delta.y)
        }
        
        Shortcut {
            enabled: map.zoomLevel < map.maximumZoomLevel
            sequence: StandardKey.ZoomIn
            onActivated: map.zoomLevel = Math.round(map.zoomLevel + 1)
        }
        
        Shortcut {
            enabled: map.zoomLevel > map.minimumZoomLevel
            sequence: StandardKey.ZoomOut
            onActivated: map.zoomLevel = Math.round(map.zoomLevel - 1)
        }
        
        // Handle map initialization
        Component.onCompleted: {
            console.log("Map plugin name:", plugin.name)
            console.log("Map plugin supported:", plugin.isAttached)
            console.log("Available map types:", supportedMapTypes.length)
            
            if (supportedMapTypes.length > 0) {
                activeMapType = supportedMapTypes[0]
            }
        }
        
        // Error handling
        onErrorChanged: {
            console.log("Map error:", error)
            console.log("Error string:", errorString)
        }
        
        // Update marker when center changes
        onCenterChanged: {
            marker.coordinate = center
            console.log("Map center changed to:", center.latitude, center.longitude)
            
            // Call C++ handler to update inputs
            if (typeof cppHandler !== 'undefined') {
                cppHandler.updateInputsFromMap(center.latitude, center.longitude)
            }
        }
    }
}