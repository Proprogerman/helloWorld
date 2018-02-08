import QtQuick 2.0
import QtQuick.Controls 2.2
//import QtQuick.Controls.Styles 1.4
//import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0

import KlimeSoft.SingletonUser 1.0

import "qrc:/qml/elements"

Page{
    id:signInPage

    property color mainColor: "#507299"
    property color backColor: "#78909C"
    property color dataColor: "#CFD8DC"
    property color errColor: "#F8BBD0"

    Connections{
        target:User
        onNameIsExist: { nameOfGroup.state = indicateZone.state = "nameIsExistState" }
        onNameIsCorrect: { nameOfGroup.state = indicateZone.state = "nameIsValidState" }
    }

    property string indicateMessage

    function signUpButtonCheck(){
        if(nameOfGroup.state == "nameIsValidState" && password.state == "passwordIsOkState")
            signUpButton.clickable = true
        else
            signUpButton.clickable = false
    }

    Rectangle{
        id: background
        anchors.fill: parent
        color: backColor
        z: -2
    }
    Item{
        id: indicateZone
        width: background.width; height: background.height * 1/3
        anchors.top: background.top
        z: -1
        state:"inputNameState"

        Text{
            id: indicateMessage
            anchors{ left: parent.left; right: parent.right; top: parent.top}
            font.family: "Impact"
            color: "lightgrey"
            font.pixelSize: parent.height * 1/6
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.WordWrap
        }
        Image{
            id: indicateImage
            height: parent.height * 3/4; width: height
            anchors{ bottom: parent.bottom; left: parent.left }
            opacity: 0.5
        }

        states: [
            State{
                name:"nameInputState"
                PropertyChanges{ target: indicateMessage;  text: "Придумайте название группы" }
                PropertyChanges{ target: indicateImage; source: "qrc:/memePhoto/mem_hmm.png" }
                PropertyChanges{ target: indicateZone; currentStateKey: "name" }
            },
            State{
                name:"nameIsValidState"
                PropertyChanges{ target: indicateMessage; text: "Название доступно" }
                PropertyChanges{ target: indicateImage; source: "qrc:/memePhoto/okMeme.png" }
                PropertyChanges{ target: indicateZone; currentStateKey: "name" }
            },
            State{
                name:"nameIsExistState"
                PropertyChanges{ target: indicateMessage; text: "Имя занято, придумайте другое" }
                PropertyChanges{ target: indicateImage; source: "qrc:/memePhoto/noMeme.png" }
                PropertyChanges{ target: indicateZone; currentStateKey: "name" }
            },

            State{
              name:"passwordInputState"
              PropertyChanges{ target: indicateMessage; text: "Придумайте пароль" }
              PropertyChanges{ target: indicateImage; source: "qrc:/memePhoto/mem_hmm.png" }
              PropertyChanges{ target: indicateZone; currentStateKey: "password" }
            },

            State{
                name:"passwordIsOkState"
                PropertyChanges{ target: indicateMessage; text: "Пароль удовлетворяет требованиям" }
                PropertyChanges{ target: indicateImage; source: "qrc:/memePhoto/okMeme.png" }
                PropertyChanges{ target: indicateZone; currentStateKey: "password" }
            },

            State{
                name:"passwordHasFewerCharsState"
                PropertyChanges{ target: indicateMessage; text: "Пароль должен быть длиннее 6-ти символов" }
                PropertyChanges{ target: indicateImage; source: "qrc:/memePhoto/noMeme.png" }
                PropertyChanges{ target: indicateZone; currentStateKey: "password" }
            }
        ]

        property string currentStateKey
        property string currentNameState: "nameInputState"
        property string currentPasswordState: "passwordInputState"

//        onStateChanged: {
//            if(currentStateKey == "name")
//                currentNameState = state
//            if(currentStateKey == "password")
//                currentPasswordState = state
//        }
    }

    Rectangle{
        id: dataSheet
        width: background.width; height: background.height * 2/3
        anchors.bottom: background.bottom
        color: dataColor

        Rectangle{
            id: nameOfGroup
            width: parent.width; height: nameInputRow.height * 2
            anchors.top: parent.top
            color:"#edeef0"

            states:[
                State{
                    name: "nameIsValidState"
                    PropertyChanges{ target: nameOfGroup; color: mainColor }
                },
                State{
                    name: "nameIsExistState"
                    PropertyChanges{ target: nameOfGroup; color: errColor }
                }
            ]

//            onStateChanged:{ indicateZone.currentNameState = state }

            TextField{
                id: nameInputRow
                width: parent.width * 3/4; height: width * 1/7;
                anchors.centerIn: parent
                //radius: height * 1/4
                font.family:"Roboto"
                font.pixelSize: height * 1/2
                placeholderText:"Название группы"
                maximumLength: 16
                validator: RegExpValidator{regExp: /^[^\s][\w\s]+$/}
                property color backgroundColor: "#ffffff"

                background: Rectangle{
                    anchors.fill: parent
                    radius: parent.height/2
                    color: parent.backgroundColor
                }

                Timer{
                    id: submitTimer
                    interval: 3000
                    running: false
                    repeat: false
                    onTriggered:{
                        if(nameInputRow.getText(0,nameInputRow.length) != '')
                            User.checkName( nameInputRow.getText(0, nameInputRow.length) )
                            signUpButtonCheck()
                        }
                }

                onActiveFocusChanged: {
                    if(activeFocus == true){
                        indicateZone.state = indicateZone.currentNameState
                        backgroundColor = "lightgrey"
                    }
                    else
                        backgroundColor = "#ffffff"
                    }
                onTextChanged:{
                    submitTimer.start()
                }
            }
        }

        Rectangle{
            id: password
            width: parent.width; height: nameInputRow.height * 2
            anchors.top: nameOfGroup.bottom
            anchors.topMargin: height * 1/20
            color:"#edeef0"

            states:[
                State{
                    name:"passwordIsOkState"
                    PropertyChanges{ target: password; color: mainColor }
                },
                State{
                    name: "passwordHasFewerCharsState"
                    PropertyChanges{ target:password; color: errColor }
                }
            ]

//            onStateChanged: { indicateZone.currentPasswordState = state }

            TextField{
                id: passwordInputRow
                width: parent.width * 3/4; height: width * 1/7
                anchors.centerIn: parent
                //radius: height * 1/4
                font.family:"Roboto"
                font.pixelSize: height * 1/2
                placeholderText:"Пароль"
                maximumLength: 16
                validator: RegExpValidator{regExp:/[a-zA-Z1-9\!\@\#\$\%\^\&\*\(\)\-\_\+\=\;\:\,\.\/\?\\\|\`\~\[\]\{\}]{6,}/}
                property color backgroundColor: "#ffffff"
                echoMode: TextInput.Password

                background: Rectangle{
                    anchors.fill: parent
                    radius: parent.height/2
                    color: parent.backgroundColor
                }

                onActiveFocusChanged: {
                    if(activeFocus == true){
                        indicateZone.state = indicateZone.currentPasswordState
                        backgroundColor = "lightgrey"
                    }
                    else
                        backgroundColor = "#ffffff"
                }
                onTextChanged:{
                    if(length < 6){
                        password.color = errColor
                        password.state = indicateZone.state = "passwordHasFewerCharsState"
                    }
                    else{
                        password.color = mainColor
                        password.state = indicateZone.state = "passwordIsOkState"
                    }
                    signUpButtonCheck()
                }

//                Rectangle{
//                    //property alias backgroundColor: color
//                    anchors.fill: parent
//                    radius: height/2
//                }

                Rectangle{
                    id: passwordVis

                    property color activeColor:"#757575"
                    property color inactiveColor:"#BDBDBD"

                    height: parent.height; width: height
                    anchors{right: parent.right; top: parent.top}
                    radius: passwordInputRow.height/2
                    color: inactiveColor
                    Rectangle{
                        height: parent.height; width: height * 1/2
                        anchors{left: parent.left; top: parent.top}
                        color: parent.color
                    }
                    MouseArea{
                        anchors.fill: parent
                        onClicked:{
                            if(passwordVis.color == passwordVis.activeColor){
                                passwordVis.color = passwordVis.inactiveColor
                                passwordInputRow.echoMode = TextInput.Password
                                }
                            else{
                                passwordVis.color = passwordVis.activeColor
                                passwordInputRow.echoMode = TextInput.Normal
                            }
                        }
                    }
                }
            }
        }
        MaterialButton{
            id: signUpButton
            width: password.width/1.5; height: password.height/1.5
            anchors{ top: password.bottom; topMargin: height/20; horizontalCenter: parent.horizontalCenter }
            label: "создать"
            radius: height/10
            clickableColor: mainColor
            unclickableColor: dataColor
            rippleColor: Qt.lighter(clickableColor, 1.5)
            clickable: false

            Connections{
                target: signUpButton.buttArea
                onClicked:{
                    console.log("signUpButton")
                    User.user_name = nameInputRow.getText(0, nameInputRow.length)
                    User.user_password = passwordInputRow.getText(0, passwordInputRow.length)

                    User.signUp()

                    User.getMemeList()

                    stackView.push(mainUserPage)
                }
            }
        }
    }
    DropShadow{
        anchors.fill: dataSheet
        horizontalOffset: 0
        verticalOffset: - indicateZone.height * 1/10
        radius: 20
        samples: 41
        color:"#80000000"
        source: dataSheet
        opacity: 0.5
    }

}
