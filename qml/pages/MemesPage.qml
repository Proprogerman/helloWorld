import VPlayApps 1.0
import QtQuick 2.0
import QtGraphicalEffects 1.0
import QtCharts 2.2

import "../elements"


Page{

    property color mainColor: "#507299"
    //property color backColor: "#78909C"
    property color backColor: "#edeef0"
    //property int spacing: pageHeader.height * 1/4

    backgroundColor: backColor

    Rectangle{
        id: pageHeader
        width: parent.width
        height: parent.height * 1/10
        anchors.top: parent.top
        color: mainColor
        z: 4

        Text{
            id: memeName
            anchors{ horizontalCenter: parent.horizontalCenter; top: parent.top;
                topMargin: height/4 }
            text: "Meme's name"//ServerConnection.user_name
            font.pixelSize: parent.height/2
        }
    }


    DropShadow{
        anchors.fill: pageHeader
        //verticalOffset: staticLine.height * 1/5
        radius: 13
        samples: 17
        color:"#000000"
        source: pageHeader
        opacity: 0.5
        z: 3
    }
    Rectangle{
        id: imageItem
        width: parent.width
        height: parent.width * 1/2
        anchors.top: pageHeader.bottom
        color: "white"
        Image {
            id: image
            width: parent.height
            height: width
            anchors.horizontalCenter: parent.horizontalCenter
            source: "../../assets/respectMeme.jpg"
        }
    }

//    Rectangle{
//        id: graphItem
//        width: parent.width
//        height: parent.height * 3/8
//        anchors.top: imageItem.bottom

//        ChartView{
//            //title: "Популярность мема:"
//            anchors.fill: parent
//            antialiasing: true

//            LineSeries{
//                XYPoint { x: 0; y: 0 }
//                XYPoint { x: 1.1; y: 2.1 }
//                XYPoint { x: 1.9; y: 3.3 }
//                XYPoint { x: 2.1; y: 2.1 }
//                XYPoint { x: 2.9; y: 4.9 }
//                XYPoint { x: 3.4; y: 3.0 }
//                XYPoint { x: 4.1; y: 3.3 }
//            }
//        }
//    }
    Pop
    Rectangle{
        id: popManipulator
        height: pageHeader.height * 3/2
        width: height
        radius: height/2
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: graphItem.bottom
        anchors.topMargin: (parent.height - graphItem.y - graphItem.height - height)/2
        color: "#EC407A"
        //
        //   состояния набора шекелькоинов и кнопки "лайк"
        //
    }
}
