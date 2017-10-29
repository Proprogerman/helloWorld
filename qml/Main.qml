import QtQuick 2.0
import QtQuick.Controls 1.4



    Item {
    // You get free licenseKeys from https://v-play.net/licenseKey
    // With a licenseKey you can:
    //  * Publish your games & apps for the app stores
    //  * Remove the V-Play Splash Screen or set a custom one (available with the Pro Licenses)
    //  * Add plugins to monetize, analyze & improve your apps (available with the Pro Licenses)

//    Loader{id:pageLoader}
//    Component.onCompleted: pageLoader.source="SignInPage.qml"
//    NewSignInPage{}

    StackView{
        id:stackView
        anchors.fill: parent
        initialItem: memesPage

//        Component{
//            id: signInPage
//            SignInPage{}
//        }

//        Component{
//            id: mainUserPage
//            MainUserPage{}
//        }

        Component{
            id: memesPage
            MemesPage{}
        }
    }
}
