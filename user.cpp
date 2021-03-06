#include <QJsonArray>
#include <QList>
#include <QVector>
#include <QVariant>
#include <QByteArray>
#include <QThread>

#include <QCryptographicHash>

#include <QImage>
#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>

#include "user.h"


User::User(QObject *parent): QObject(parent)
{
    connect(&clientSocket, &QTcpSocket::readyRead, this, &User::onReadyRead);
    connect(&clientSocket, &QTcpSocket::disconnected, this, &User::onDisconnected);

    if(!settings.value("user/name").toString().isEmpty())
        user_name = settings.value("user/name").toString();

    connect(this, &User::signAnswered, this, &User::storeUserSettings);
}

void User::checkName(const QString &name){
    QJsonObject jsonObj
    {
        {"requestType", "checkName"},
        {"user_name", name}
    };
    writeData(QJsonDocument(jsonObj).toBinaryData());
}

void User::setName(const QString &name){
    user_name = name;
    emit nameChanged();
}

void User::setPasswordHash(const QString &hash){
    passwordHash = hash;
}

void User::setUserData(const QJsonObject &userData)
{
    QString imgName = userData["imageName"].toString();
    if(imgName != user_imageName){
        user_imageName = imgName;
        emit imageReceived("user", getName(), imgName);
    }
    removeExcessImages({imgName}, "user");
    int tempPopValue = userData["pop_value"].toInt();
    int tempCreativity = userData["creativity"].toInt();
    int tempShekels = userData["shekels"].toInt();

    if(pop_value != tempPopValue){
        pop_value = tempPopValue;
        emit popValueChanged();
    }
    if(creativity != tempCreativity){
        creativity = tempCreativity;
        emit creativityChanged();
    }
    if(shekels != tempShekels){
        shekels = tempShekels;
        emit shekelsChanged();
    }
    QVariantList memeList = userData["memeList"].toArray().toVariantList();
    QVector<QString> memeNames;
    QVector<QString> imageNames;
    for(int i = 0; i < memeList.size(); i++)
    {
        QVariantMap memeObj = memeList[i].toMap();
        QString memeName = memeObj["memeName"].toString();
        memeNames.append(memeName);
        QVariantList tempPop = memeObj["popValues"].value<QVariantList>();
        const int startPopValue = memeObj["startPopValue"].toInt();
        const int creativity = memeObj["creativity"].toInt();
        const int loyalty = memeObj["loyalty"].toInt();
        const QString category = memeObj["category"].toString();
        QString imageName = memeObj["imageName"].toString();
        imageNames.append(imageName);
        QVector<int> memeValues;
        for(int i = 0; i < tempPop.size(); i++){
            memeValues.push_back(tempPop[i].toInt());
        }
        setMeme(memeName, memeValues, imageName, category, loyalty, creativity, true, startPopValue);
        if(!memeObj["imageUrl"].isNull())
            setMemeImage(QJsonObject::fromVariantMap(memeObj));
    }
    for(int i = 0; i < memes.size(); i++)
        if(!memeNames.contains(memes[i].getName()))
            removeMeme(i, true);
}

bool User::findMeme(const QString &name){
    foreach(Meme memeCont, memes)
        if(memeCont.getName() == name)
            return memeCont.getForced();
    return false;
}

void User::setMemeImage(const QJsonObject &jsonObj)
{
    QString memeName = jsonObj["memeName"].toString();
    QString imageName = jsonObj["imageName"].toString();
    QString category = jsonObj["category"].toString();
    QUrl imageUrl = QUrl(jsonObj["imageUrl"].toString());
    QNetworkReply *reply = mngr.get(QNetworkRequest(imageUrl));
    connect(reply, &QNetworkReply::finished, [=](){ getImageFromVk(reply, "meme", memeName, imageName, category);});
}

void User::setAdImage(const QJsonObject &jsonObj)
{
    QString adName = jsonObj["adName"].toString();
    QString imageName = jsonObj["imageName"].toString();
    QByteArray encoded = jsonObj["imageData"].toString().toLatin1();
    QImage adImage;
    adImage.loadFromData(QByteArray::fromBase64(encoded), "PNG");
    QString homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
    QDir imgs(homePath + "/imgs/ad");
    if(!imgs.exists())
        imgs.mkpath(imgs.path());
    adImage.save(imgs.path() + '/' + imageName, "PNG");
    emit imageReceived("ad", adName, imageName);
}

void User::setUserImage(const QJsonObject &jsonObj)
{
    QString imageName = jsonObj["imageName"].toString();
    QByteArray encoded = jsonObj["imageData"].toString().toLatin1();
    QImage userImage;
    userImage.loadFromData(QByteArray::fromBase64(encoded), "PNG");
    QString homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
    QDir imgs(homePath + "/imgs/user");
    if(!imgs.exists())
        imgs.mkpath(imgs.path());
    userImage.save(imgs.path() + '/' + imageName, "PNG");
    emit imageReceived("user", getName(), imageName);
}

void User::setMemesWithCategory(const QVariantList &memeList, const QString &category)
{
    QVector<QString> memeNames, imageNames;
    for(int i = 0; i < memeList.size(); i++)
    {
        QVariantMap memeObj = memeList[i].toMap();
        QString memeName = memeObj["memeName"].toString();
        memeNames.append(memeName);
        QVariantList tempPop = memeObj["popValues"].value<QVariantList>();
        int loyalty = memeObj["loyalty"].toInt();
        int creativity = memeObj["creativity"].toInt();
        bool forced = memeObj["forced"].toBool();
        int startPopValue = memeObj["startPopValue"].toInt();
        QString imageName = memeObj["imageName"].toString();
        imageNames.append(imageName);
        memeObj["category"] = category;
        QVector<int> memeValues;
        for(int i = 0; i < tempPop.size(); i++){
            memeValues.push_back(tempPop[i].toInt());
        }
        setMeme(memeName, memeValues, imageName, category, loyalty, creativity, forced, startPopValue);
        if(!memeObj["imageUrl"].isNull())
            setMemeImage(QJsonObject::fromVariantMap(memeObj));
    }
    for(int i = 0; i < memes.size(); i++)
        if(!memeNames.contains(memes[i].getName()) && memes[i].getCategory() == category)
            removeMeme(i, false);
    removeExcessImages(imageNames, "meme", category);
}

void User::setAdList(const QVariantList &adList)
{
    QVector<QString> adNames, imageNames;
    for(int i = 0; i < adList.size(); i++){
        QVariantMap adObj = adList[i].toMap();
        QString adName = adObj["adName"].toString();
        adNames.append(adName);
        QString imageName = adObj["imageName"].toString();
        imageNames.append(imageName);
        QString adReputation = adObj["reputation"].toString();
        int adProfit = adObj["profit"].toInt();
        int adDiscontented = adObj["discontented"].toInt();
        int adSecondsToReady = adObj["secondsToReady"].toInt();
        int indexOfAd = getAdIndex(adName);
        if(indexOfAd != -1)
            ads.replace(indexOfAd, Ad(adName, imageName, adReputation, adProfit, adDiscontented, adSecondsToReady));
        else
            ads.append(Ad(adName, imageName, adReputation, adProfit, adDiscontented, adSecondsToReady));
        emit adReceived(adName, imageName, adReputation, adProfit, adDiscontented, adSecondsToReady);
    }
    for(int i = 0; i < ads.size(); i++)
        if(!adNames.contains(ads[i].getName()))
            removeAd(ads[i].getName());
    removeExcessImages(imageNames, "ad");
}

void User::setMemeData(const QJsonObject &obj)
{
    QVariantList tempPop = obj["popValues"].toArray().toVariantList();
    const QString memeName = obj["memeName"].toString();
    const double loyalty = obj["loyalty"].toDouble();
    const QString category = obj["category"].toString();
    const int creativity = obj["creativity"].toInt();
    const bool forced = obj["forced"].toBool();
    const int startPopValue = obj["startPopValue"].toInt();
    QVector<int> memeValues;
    for(int i = 0; i < tempPop.size(); i++){
        memeValues.push_back(tempPop[i].toInt());
    }
    setMeme(memeName, memeValues, "null", category, loyalty, creativity, forced, startPopValue);
}

void User::setUsersRating(const QJsonArray &userList, const int &userRating)
{
    emit usersRatingReceived(userList.toVariantList(), userRating);
}

void User::setMeme(const QString &memeName, const QVector<int> &memeValues, const QString &memeImageName,
                   const QString &memeCategory, const int &memeLoyalty, const int &creativity, const bool &forced,
                   const int &memeStartPopValue)
{
    for(int i = 0; i < memes.size(); i++){
        if(memes[i].getName() == memeName){
            memes[i].setPopValues(memeValues);
            memes[i].setImageName(memeImageName == "null" ? memes[i].getImageName() : memeImageName);
            memes[i].setCategory(memeCategory);
            memes[i].setLoyalty(memeLoyalty);
            memes[i].setCreativity(creativity);
            memes[i].setForced(forced);
            memes[i].setStartPopValue(memeStartPopValue);
            emit memeReceived(memeName, memeValues, memeLoyalty, memeCategory, memeImageName, creativity, memeStartPopValue);
            return;
        }
    }
    memes.append(Meme(memeName, memeValues, memeImageName, memeCategory, memeLoyalty, creativity, forced, memeStartPopValue));
    emit memeReceived(memeName, memeValues, memeLoyalty, memeCategory, memeImageName, creativity, memeStartPopValue);
}

void User::removeMeme(const int &index, const bool &mine)
{
    const QString memeName = memes[index].getName();
    if(!mine){
        memes.remove(index);
        emit memeRemoved(memeName, memes[index].getCategory());
    }
    else{
        memes[index].setForced(false);
        emit memeUnforced(memeName);
    }
}

void User::removeAd(const QString &adName)
{
    for(int i = 0; i < ads.size(); i++)
        if(ads[i].getName() == adName){
            ads.remove(i);
            emit adRemoved(adName);
        }
}

bool User::findCategoryMeme(const QString &name, const QString &category){
    foreach(Meme memeCont, memes){
        if(memeCont.getCategory() == category && memeCont.getName() == name)
            return true;
    }
    return false;
}

bool User::findAd(const QString &name)
{
    foreach(Ad adCont, ads){
        if(adCont.getName() == name)
            return true;
    }
    return false;
}

QString User::getConfData(const QString &fileName, const QString &category, const QString &dataName)
{
    const QString filePath = ":/conf/" + fileName;
    QFile confData(filePath);
    confData.open(QFile::ReadOnly | QIODevice::Text);
    QString data = confData.readAll();
    confData.close();
    QJsonObject obj = QJsonDocument::fromJson(data.simplified().toUtf8()).object()
            .value(category).toObject();
    return obj[dataName].toString();
}

int User::getAdIndex(const QString &name)
{
    for(int i = 0; i < ads.size(); i++){
        if(ads[i].getName() == name)
            return i;
    }
    return -1;
}

QString User::getMemeCategory(const QString &imageName)
{
    for(Meme el : memes){
        if(el.getImageName() == imageName){
            return el.getCategory();
        }
    }
    return "";
}

void User::rewardUserWithShekels()
{
    const int shekelsReward = 100;

    QJsonObject jsonObj {
                            {"requestType", "rewardUserWithShekels"},
                            {"user_name", getName()},
                            {"shekels", shekelsReward}
                        };

    writeData(QJsonDocument(jsonObj).toBinaryData());
    shekels += shekelsReward;
    emit shekelsChanged();
}

QString User::hashPassword(const QString &password, const QString &login){
    QByteArray passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha224);

    for(int i = 0; i < 4; i++){
        passwordHash = QCryptographicHash::hash(login.toUtf8() + passwordHash, QCryptographicHash::Sha224);
    }

    return QString::fromStdString(passwordHash.toHex().toStdString());
}

void User::connectToHost()
{
    if(clientSocket.state() != QTcpSocket::ConnectedState){
        int port = getConfData("appData.json", "Server", "port").toInt();
        clientSocket.connectToHost(getConfData("appData.json", "Server", "host"), port);
        clientSocket.waitForConnected();
    }
}

bool User::writeData(const QByteArray &data)
{
    connectToHost();
    if(clientSocket.state() == QTcpSocket::ConnectedState){
        clientSocket.write(intToArray(data.size()));
        clientSocket.write(data);
        requestTimer.singleShot(5000, this, &User::resetRequest);
        return clientSocket.waitForBytesWritten();
    }
    else
        return false;
}

quint32 User::arrayToInt(QByteArray dataSize)
{
    quint32 temp;
    QDataStream stream(&dataSize, QIODevice::ReadWrite);
    stream >> temp;
    return temp;
}

QByteArray User::intToArray(const quint32 &dataSize)
{
    QByteArray temp;
    QDataStream stream(&temp, QIODevice::ReadWrite);
    stream << dataSize;
    return temp;
}

QJsonArray User::getLocalImagesList(const QString &type, QString category)
{
    QString homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
    QString path = homePath + "/imgs/" + type;
    if(type == "meme"){
        if(!category.isEmpty())
            path.append('/' + category);
        else{
            QJsonArray imgArr;
            QDirIterator it(path, QStringList({"*.jpg"}), QDir::Files, QDirIterator::Subdirectories);
            while(it.hasNext()){
                QFileInfo fil(it.next());
                imgArr.append(fil.fileName());
            }
            return imgArr;
        }
    }
    QDir imgs(path);
    return QJsonArray::fromStringList(imgs.entryList());
}

void User::removeExcessImages(const QVector<QString> &images, const QString &type, QString category)
{
    QJsonArray localImages = getLocalImagesList(type, category);
    QString imgs = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0] + "/imgs/";
    for(QJsonValue el : localImages)
        if(!images.contains(el.toString())){
            QString path = imgs + type + '/';
            if(type == "meme" && !category.isEmpty()){
                path.append(category + "/");
            }
            QFile::remove(path + el.toString());
        }
}

void User::signUp(const QString &name, const QString &password)
{
    passwordHash = hashPassword(password, name);

    QJsonObject jsonObj {
                            {"requestType", "signUp"},
                            {"user_name", name},
                            {"passwordHash", passwordHash}
                        };

    writeData(QJsonDocument(jsonObj).toBinaryData());
}

void User::signIn(const QString &name, const QString &password)
{
    passwordHash = hashPassword(password, name);

    QJsonObject jsonObj {
                            {"requestType", "signIn"},
                            {"user_name", name},
                            {"passwordHash", passwordHash}
                        };

    writeData(QJsonDocument(jsonObj).toBinaryData());
}

void User::autoSignIn()
{
    passwordHash = settings.value("user/passwordHash").toString();

    QJsonObject jsonObj {
                            {"requestType", "signIn"},
                            {"user_name", settings.value("user/name").toString()},
                            {"passwordHash", settings.value("user/passwordHash").toString()}
                        };

    writeData(QJsonDocument(jsonObj).toBinaryData());
}

void User::signOut()
{
    QJsonObject jsonObj {
                            {"requestType", "signOut"},
                            {"user_name", getName()}
    };

    settings.setValue("user/name", "");
    settings.setValue("user/passwordHash", "");
    settings.sync();
    setName("");
    setPasswordHash("");
    pop_value = 0;
    creativity = 0;
    shekels = 0;
    user_imageName = "";
    memes.clear();

    writeData(QJsonDocument(jsonObj).toBinaryData());
}

void User::getUserData()
{
    QJsonArray resultList = getLocalImagesList("user");
    QJsonArray memesImagesList = getLocalImagesList("meme");
    for(QJsonValue el : memesImagesList)
        resultList.append(el);

    QJsonObject jsonObj {
                            {"requestType", "getUserData"},
                            {"user_name", getName()},
                            {"localImages", resultList},
                            {"screenWidth", settings.value("device/screen/width").toString()},
                            {"screenHeight", settings.value("device/screen/height").toString()}
                        };

    writeData(QJsonDocument(jsonObj).toBinaryData());
}

void User::setExistingCategoriesList()
{
    emit memesCategoriesReceived(categories);
}

void User::onReadyRead(){
    while(clientSocket.bytesAvailable()){
        QByteArray byteArr;

        quint32 dataSize = arrayToInt(clientSocket.read(sizeof(quint32)));
        byteArr = clientSocket.read(dataSize);

        while(byteArr.size() < dataSize){
            clientSocket.waitForReadyRead();
            byteArr.append(clientSocket.read(dataSize - byteArr.size()));
        }

        QJsonObject jsonObj = QJsonDocument::fromBinaryData(byteArr).object();

        processingResponse(jsonObj);
    }
    clientSocket.disconnectFromHost();
}

void User::onDisconnected(){
    clientSocket.close();
}

void User::storeUserSettings(QString name, bool isSigned){
    if(isSigned){
        settings.setValue("user/name", name);
        settings.setValue("user/passwordHash", passwordHash);
        settings.sync();
    }
}

void User::getImageFromVk(QNetworkReply *reply, QString type, QString itemName, QString imageName,
                          QString category)
{
    QImage image;
    image.loadFromData(reply->readAll());
    QString homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
    QDir imgs(homePath + "/imgs/meme/" + category + '/');
    if(!imgs.exists())
        imgs.mkpath(imgs.path());
    image.save(imgs.path() + '/' + imageName, "JPG");
    emit imageReceived(type, itemName, imageName);
    delete reply;
}

void User::resetRequest()
{
    clientSocket.disconnectFromHost();
}

QString User::getName(){
    return user_name;
}

int User::getUserPopValue(){
    return pop_value;
}

int User::getCreativity(){
    return creativity;
}

int User::getShekels(){
    return shekels;
}

QString User::getImageName()
{
    return user_imageName;
}

void User::processingResponse(QJsonObject &jsonObj)
{
    const QString responseType = jsonObj["responseType"].toString();

    if(responseType == "checkNameResponse"){
        if(jsonObj["nameAvailable"].toBool()){
            emit nameAvailabilityChanged(true, jsonObj["name"].toString());
        }
        else{
            emit nameAvailabilityChanged(false, jsonObj["name"].toString());
        }
    }
    else if(responseType == "signUpResponse"){
        emit signAnswered(jsonObj["user_name"].toString(), jsonObj["created"].toBool());
    }
    else if(responseType == "signInResponse"){
        emit signAnswered(jsonObj["user_name"].toString(), jsonObj["accessed"].toBool());
    }
    else if(responseType == "getUserDataResponse"){
        setUserData(jsonObj);
    }
    else if(responseType == "getMemeDataResponse"){
        setMemeData(jsonObj);
    }
    else if(responseType == "userImageResponse"){
        setUserImage(jsonObj);
    }
    else if(responseType == "adImageResponse"){
        setAdImage(jsonObj);
    }
    else if(responseType == "getMemeListWithCategoryResponse"){
        setMemesWithCategory(jsonObj["memeList"].toArray().toVariantList(), jsonObj["category"].toString());
    }
    else if(responseType == "getAdListResponse"){
        setAdList(jsonObj["adList"].toArray().toVariantList());
    }
    else if(responseType == "getMemesCategoriesResponse"){
        categories = jsonObj["categories"].toArray().toVariantList();
        emit memesCategoriesReceived(categories);
    }
    else if(responseType == "getUsersRatingResponse"){
        setUsersRating(jsonObj["usersList"].toArray(), jsonObj["user_rating"].toInt());
    }
}


QObject* User::qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine){
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return getInstance();
}

User* User::instance = nullptr;

User* User::getInstance(){
    if(instance == nullptr){
        instance = new User();
    }
    return instance;
}

void User::setExistingMemeListWithCategory(const QString &category)
{
    foreach(Meme memeCont, memes){
        if(memeCont.getCategory() == category)
            emit memeReceived(memeCont.getName(), memeCont.getPopValues(), memeCont.getLoyalty(), category,
                                memeCont.getImageName(), memeCont.getCreativity(), memeCont.getStartPopValue());
    }
}

void User::setExistingAdList()
{
    foreach(Ad adCont, ads){
        emit adReceived(adCont.getName(), adCont.getImageName(), adCont.getReputation(), adCont.getProfit(),
                        adCont.getDiscontented(), adCont.getSecondsToReady());
    }
}

void User::getMemeListWithCategory(const QString &category)
{

    QJsonObject jsonObj {
                            {"requestType", "getMemeListWithCategory"},
                            {"user_name", getName()},
                            {"category", category},
                            {"localImages", getLocalImagesList("meme", category)},
                            {"screenWidth", settings.value("device/screen/width").toString()},
                            {"screenHeight", settings.value("device/screen/height").toString()}
    };
    writeData(QJsonDocument(jsonObj).toBinaryData());
}

void User::getAdList()
{
    QJsonObject jsonObj {
                            {"requestType", "getAdList"},
                            {"localImages", getLocalImagesList("ad")},
                            {"user_name", getName()},
                            {"lang", settings.value("user/language").toString()}
    };
    writeData(QJsonDocument(jsonObj).toBinaryData());
}

void User::getMemeData(const QString &memeName)
{
    QJsonObject jsonObj {
                            {"requestType", "getMemeData"},
                            {"meme_name", memeName},
                            {"user_name", getName()}
                        };

    writeData(QJsonDocument(jsonObj).toBinaryData());
}

void User::getMemesCategories()
{
        QJsonObject jsonObj {
                                {"requestType", "getMemesCategories"},
                                {"user_name", getName()}
                            };

        writeData(QJsonDocument(jsonObj).toBinaryData());
}

void User::getUsersRating()
{
    QJsonObject jsonObj {
                            {"requestType", "getUsersRating"},
                            {"user_name", getName()}
                        };

    writeData(QJsonDocument(jsonObj).toBinaryData());
}

void User::forceMeme(const QString &memeName, const int &contributedCreativity, const int &startPopValue)
{
    QJsonObject jsonObj {
                            {"requestType", "forceMeme"},
                            {"meme_name", memeName},
                            {"user_name", getName()},
                            {"startPopValue", startPopValue},
                            {"creativity", contributedCreativity}
                        };
    writeData(QJsonDocument(jsonObj).toBinaryData());
    for(int i = 0; i < memes.size(); i++){
        if(memes[i].getName() == memeName){
            memes[i].setForced(true);
            memes[i].setStartPopValue(startPopValue);
        }

    }

    creativity -= contributedCreativity;
    emit creativityChanged();
}

void User::unforceMeme(const QString &memeName)
{
    QJsonObject jsonObj {
                            {"requestType", "unforceMeme"},
                            {"meme_name", memeName},
                            {"user_name", getName()}
                        };
    writeData(QJsonDocument(jsonObj).toBinaryData());
    for(int i = 0; i< memes.size(); i++){
        if(memes[i].getName() == memeName){
            memes[i].setForced(false);
            memes[i].setStartPopValue(-1);
            emit memeUnforced(memeName);
            break;
        }
    }
}

void User::increaseLikesQuantity(const QString &memeName, const int &investedShekels)
{
    int indexOfMeme;
    for(int j = 0; j < memes.size(); j++){
        if(memes[j].getName() == memeName){
            indexOfMeme = j;
            break;
        }
    }
    QVector<int> memeValues = memes[indexOfMeme].getPopValues();
    QVariantList variantValues;
    foreach(int value, memeValues){
        variantValues.append(value);
    }
    QJsonObject jsonObj {
                            {"requestType", "increaseLikesQuantity"},
                            {"meme_name", memeName},
                            {"currentPopValues", QJsonArray::fromVariantList(variantValues)},
                            {"shekels", investedShekels},
                            {"user_name", getName()}
                        };
    writeData(QJsonDocument(jsonObj).toBinaryData());

    const int likeIncrement = memeValues.last() + investedShekels;

    if(memeValues.size() == 12){
        for(int i = 0; i < memeValues.size() - 1; i++){
            memeValues[i] = memeValues[i + 1];
        }
        memeValues.last() = likeIncrement;
    }
    else{
        memeValues.append(likeIncrement);
    }
    memes[indexOfMeme].setPopValues(memeValues);
    emit memeReceived(memes[indexOfMeme].getName(), memeValues, memes[indexOfMeme].getLoyalty(),
                      memes[indexOfMeme].getCategory(), memes[indexOfMeme].getImageName(), memes[indexOfMeme].getCreativity(),
                      memes[indexOfMeme].getStartPopValue());
    shekels -= investedShekels;
    emit shekelsChanged();
}

void User::acceptAd(const QString &adName)
{
    Ad  tempAd;
    foreach(Ad adCont, ads){
        if(adCont.getName() == adName){
            tempAd = adCont;
            break;
        }
    }
    QJsonObject jsonObj {
                            {"requestType", "acceptAd"},
                            {"user_name", getName()},
                            {"adName", adName},
                            {"adProfit", tempAd.getProfit()},
                            {"adDiscontented", tempAd.getDiscontented()}
                        };
    writeData(QJsonDocument(jsonObj).toBinaryData());
}

bool User::categoriesIsEmpty()
{
    categories.isEmpty();
}

bool User::memesWithCategoryIsEmpty(const QString &category){
    foreach(Meme memeCont, memes)
        if(memeCont.getCategory() == category)
            return false;
    return true;
}

bool User::adsIsEmpty()
{
    return ads.isEmpty();
}

void User::localUpdateUserData()
{
    foreach(Meme memeCont, memes){
        emit memeReceived(memeCont.getName(), memeCont.getPopValues(), memeCont.getLoyalty(),
                          memeCont.getCategory(), memeCont.getImageName(), memeCont.getCreativity(), memeCont.getStartPopValue());
    }
    emit creativityChanged();
    emit shekelsChanged();
    emit popValueChanged();
}

void User::localUpdateMeme(const QString &memeName)
{
    foreach(Meme memeCont, memes){
        if(memeCont.getName() == memeName){
            emit memeReceived(memeName, memeCont.getPopValues(), memeCont.getLoyalty(), memeCont.getCategory(),
                              memeCont.getImageName(), memeCont.getCreativity(), memeCont.getStartPopValue());
            return;
        }
    }
}

