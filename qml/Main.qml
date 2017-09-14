import VPlayApps 1.0
import QtQuick 2.0
import QtQuick.Controls 1.4

import "pages"

App {
    // You get free licenseKeys from https://v-play.net/licenseKey
    // With a licenseKey you can:
    //  * Publish your games & apps for the app stores
    //  * Remove the V-Play Splash Screen or set a custom one (available with the Pro Licenses)
    //  * Add plugins to monetize, analyze & improve your apps (available with the Pro Licenses)
    licenseKey: "E63FEAE5D9780630A57D1EF6F7F248F7DFD1FA820A2B1D71362F09490013AE64E06E5965F8AE2CE
    F35ABEDF6FD567894C892F28C0FE1FCCD13E437C877A78C0C4F284551BC11049D44D373CC37C3733AA918CDF19DA
    F52EE72BB5F210ACB21BD74480C4F90066ABDA96B88258CD854D9DF4609D39A3B42860E1EDFF3BC676D40BCCB04B
    65BC96B1FA372C31D920917E2239C55A801A4CA8F525824E9C410C89FBD48E931EACA69BE5C925A7AA24B3E3FAD9
    E629B15EAA144197DB6002F0E6BAFCEB8C00DEF6C87F3BFCBDB8500FE019CA05D73D4B0D406C5671B01E4E014AD7
    4498EA12AC69B714968B289C81140FA5FA3F858FE0F4F2F0C9CDF262BB696747D2DF5718D9E548F2AD73181337F4
    06AFDD2AB69BB76D9E9ED5F54E95C7F94102545B26283EC337FDD9E8FDC16D3E046EDF65970B92954F762A8179C3
    B5B12CB3B1B337A70C3472FFD0FD70E250BF88E0E56D31508A0EF2E6D48454DB5D00AC695"

//    Loader{id:pageLoader}
//    Component.onCompleted: pageLoader.source="SignInPage.qml"
//    NewSignInPage{}

    StackView{
        id:stackView
        anchors.fill: parent
        initialItem: mainUserPage

        Component{
            id: newSignInPage
            NewSignInPage{}
        }

        Component{
            id: mainUserPage
            MainUserPage{}
        }
    }
}