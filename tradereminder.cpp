#include "tradereminder.h"
#include "ui_tradereminder.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QSet>
#include <QMap>
#include <QFileDialog>
#include <QClipboard>
#include <stdio.h>
#include <direct.h>
#include <QDebug>


bool isFileExist(QString fullFileName){
    QFileInfo fileInfo(fullFileName);
    if(fileInfo.isFile()){
        return true;
    }
    return false;
}

QString formatInfo(QString usr, int n_agents){
     QString info = "当前用户：";
     info += usr;
     info += "\n";
     info += "机构库已记录数量：" + QString::number(n_agents);

     return info;
}

QString cleanAgent(QString &agent){
    agent.simplified();
    if(agent.startsWith("(") && agent.endsWith(")")){
        agent.remove(0,1);
        agent.chop(1);
    }

    if(agent.startsWith("桥:")){
        agent.remove(0,2);
    }

    return agent;
}


bool findAgent(QString line, QString &trader1, QString &trader2){
    bool anaTrader = true;
    QStringList sec = line.split("DVP");
    if(sec.length()!=2){
        anaTrader = false;
    }
    else{
        QString traders = sec.at(1);
        QStringList traderPair = traders.split("to");
        if(traderPair.length()!=2){
            anaTrader = false;
        }
        else{
            trader1 = cleanAgent(traderPair[0]);
            trader2 = cleanAgent(traderPair[1]);

        }
    }

    return anaTrader;
}



bool loadLib(QString &fpath, QSet<QString> &S){
    QFile file(fpath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString sline = line;
        sline.simplified();
        if(sline==""){
            continue;
        }
        S.insert(sline);
    }
    file.close();
    return true;

}

tradeReminder::tradeReminder(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::tradeReminder)
{
    ui->setupUi(this);
    //get cwd
    char *buffer = getcwd(NULL,0);
    currentDir = QString::fromUtf8(buffer);
    currentUser = "琪琪";
    userAgentFile = currentDir + "\\" + currentUser + ".agents";
    qDebug()<<currentDir;
    qDebug()<<userAgentFile;
    //check if default dir exists, if not, create it.
    if(!isFileExist(userAgentFile)){
        QFile file(userAgentFile);
        if(!file.open(QFile::WriteOnly | QFile::Text)){
           throw QString("创建默认用户文件失败");
        }
        else{
            file.close();
        }
    }
    this->do_login(currentUser, userAgentFile);
    this->ui->userNameEdit->clear();
    
}

tradeReminder::~tradeReminder()
{
    delete ui;
}

void tradeReminder::do_login(QString &userName, QString &userAgentFile){
    this->userAgentFile = userAgentFile;
    this->currentUser = userName;
    QSet<QString> lib;
    bool success = loadLib(this->userAgentFile, lib);
    if(!success){
        return;
    }
    this->n_agents = lib.count();
    QString info = formatInfo(this->currentUser, this->n_agents);
    this->ui->infoLabel->setText(info);
}




void tradeReminder::useUser(){
   QString s = ui->userNameEdit->text();
   QString suppose_userAgentFile = this->currentDir + "\\" + s + ".agents";
   if(isFileExist(suppose_userAgentFile)){
       this->do_login(s,suppose_userAgentFile);
   }
   else{
       QMessageBox::StandardButton status = QMessageBox::information(NULL,"提示","该用户尚未创建，是否创建？",QMessageBox::Yes|QMessageBox::No);
       if(status==QMessageBox::Yes){
           //create agent file
           QFile file(suppose_userAgentFile);
           if(!file.open(QFile::WriteOnly | QFile::Text)){
              throw QString("创建用户文件失败");
           }
           else{
               file.close();
           }

           // do login
           this->do_login(s, suppose_userAgentFile);

       }
       else{
           QString info(formatInfo("未注册用户", 0));
           this->ui->infoLabel->setText(info);
       }
   }
}



void tradeReminder::addToLib(){
    if(this->currentUser==""){
        QMessageBox::critical(NULL,"Error","请设置当前用户");
        return;
    }

    QString infos = this->ui->agentEdit->toPlainText();
    // we have + - operators.
    // +: add this agent to lib, default
    // -: delete this agent

    //load from agent file, to get all
    QSet<QString> lib;
    bool flag = loadLib(this->userAgentFile, lib);
    if(!flag){
        return;
    }
    //split infos to lines by \n
    QStringList lines = infos.split("\n");
    QString tmp;
    int agent_diff = 0;
    for(int i=0;i<lines.length();i++){
        tmp = lines[i];
        if(tmp=="" || tmp=="\n"){
            continue;
        }

        //toCheck: make sure tmp is not endswith '\n'
        if(tmp.startsWith('+')){

            //remove +
            tmp.remove(0,1);
            //qDebug()<<"A add operation detecetd";
            if(!lib.contains(tmp)){
                lib.insert(tmp);
            }
            agent_diff += 1;
            //qDebug()<<"lib added " + tmp;

        }
        else if(tmp.startsWith("-")){
            tmp.remove(0,1);
            //qDebug()<<"A remove operation detected";
            if(lib.contains(tmp)){
                lib.remove(tmp);
            }
            //qDebug()<<"lib removed " + tmp;
            agent_diff -=1;
        }
    //do nothing for no operator.
    //perhaps we should clear processed lines only.
    this->ui->agentEdit->clear();
    }

    //write data to file
     QFile file(this->userAgentFile);
     if (!file.open(QFile::WriteOnly | QFile::Text)){
         return;
     }
     else{
         QSetIterator<QString> iter(lib);
         while(iter.hasNext()){
             file.write((iter.next().simplified() + "\n").toUtf8());
         }
     }
     file.close();

     // update agent count
     this->n_agents += agent_diff;
     QString info = formatInfo(this->currentUser, this->n_agents);
     this->ui->infoLabel->setText(info);

}


void tradeReminder::showAgents(){
    //just show data in Lib
    QSet<QString> lib;
    bool success = loadLib(this->userAgentFile, lib);
    if(!success){
        return;
    }
    this->ui->agentEdit->clear();
    QSetIterator<QString> iter(lib);
    while(iter.hasNext()){
        this->ui->agentEdit->append(iter.next().simplified());
    }

}


void tradeReminder::importData(){
    QString fileName= QFileDialog::getOpenFileName(this,tr("Open file"), "./", tr("txt Files (*.txt *.csv)"));
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
      return;
    }
    QString content;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        content += line;
    }
    file.close();
    //show contents on windows
    this->ui->inputZoneEdit->setText(content);
}


void tradeReminder::exportData(){
    QString fileName= QFileDialog::getSaveFileName(this,tr("Save file"), "./", tr("txt Files (*.txt *.csv)"));
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
      return;
    }


    //write everhthing to the file
    QString content = this->ui->resShowBrowser->toPlainText();
    file.write(content.toStdString().data());
    file.close();
}

void tradeReminder::copyRes(){
   QClipboard *clipboard = QGuiApplication::clipboard();
   QString content = this->ui->resShowBrowser->toPlainText();
   clipboard->setText(content);
}


void tradeReminder::doRefresh(){
    // read lines in input Zone
    QString content = this->ui->inputZoneEdit->toPlainText();
    
    // nothing to process, exit
    if(content.size() < 10){
        return;
    }
    
    QStringList lines = content.split("\n");
    //qDebug()<<"info read: \n" << lines;
    QSet<QString> poss_new_agents;
    QMap<QString,QStringList> m;
    QStringList tmp;
    m["Unknown"] = tmp;
    
    // read current agents
    QFile file(this->userAgentFile);
    QSet<QString> agents;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString sline = line;
        sline.simplified();
        if(sline==""){
            continue;
        }
        agents.insert(sline);
        //qDebug()<<"Read agents: "<< sline;
    }
    file.close();

    //loop over input lines to find corresponding agent.
    QString currentLine;

    for(int i=0;i<lines.length();i++){
        currentLine = lines[i];
        if(currentLine.length()<10){
            continue;
        }

        QString trader1;
        QString trader2;
        bool flag = findAgent(currentLine,trader1,trader2);
        bool inlab = false;
        QSetIterator<QString> iter(agents);
        QString item;
        while(iter.hasNext()){
            item = iter.next().simplified();

            // currentline contains user;
            //qDebug()<<"Current Line： "<<currentLine;
            //qDebug()<<"targe item " << item;
            //qDebug()<<"match result: "<<currentLine.contains(item);
            if(currentLine.contains(item)){
                inlab=true;
                //qDebug()<<"Find a agent for info " << item;
                if(m.contains(item)){
                    m[item].push_back(currentLine);
                    //qDebug() << "Add info to this item"<<endl;
                }
                else{
                    QStringList currentList;
                    currentList.push_back(currentLine);
                    m[item] = currentList;
                    //qDebug()<<"create slot for this item"<<endl;
                }

            }

        }
        //qDebug()<<"Inlab flag: "<<inlab;
        //above loop find nothing, set to unknowns
        if(!inlab){
            m["Unknown"].push_back(currentLine);
            if(!flag){
                poss_new_agents.insert(trader1.simplified());
                poss_new_agents.insert(trader2.simplified());
                //qDebug()<<"Find agents: " << trader1 << "\n" << trader2<<endl;
            }
        }

        //process and format for writting.
        QStringList writeOut;
        QString key;
        QStringList value;
        QMapIterator<QString, QStringList> it(m);
        QStringList unknownList = m["Unknown"];

        writeOut.push_back("Unknown:");
        for(int u=0;u<unknownList.length();u++){
            unknownList[u].simplified();
            QStringList frags = unknownList[u].split(" ");
            frags[0] = QString::number(u+1);
            QString new_l = frags.join(" ");
            //qDebug()<<"New result: "<<key<<":"<<new_l<<endl;
            writeOut.push_back(new_l);
        }

        while(it.hasNext()){
            it.next();
            key = it.key();
            //skip Unknown
            if(key=="Unknown"){
                continue;
            }
            value = it.value();
            writeOut.push_back("\n" + key + ":");
            for(int j=0;j<value.length();j++){
                value[j].simplified();
                QStringList frags = value[j].split(" ");
                frags[0] = QString::number(j+1);
                QString new_l = frags.join(" ");
                //qDebug()<<"New result: "<<key<<":"<<new_l<<endl;
                writeOut.push_back(new_l);
            }
        }

        QString outText =writeOut.join("\n");

        QString outAgent = "";
        QSetIterator<QString> agentIter(poss_new_agents);
        while(agentIter.hasNext()){
            outAgent += agentIter.next().simplified() + "\n";
        }

        //display
        this->ui->resShowBrowser->clear();
        this->ui->resShowBrowser->setText(outText);
        this->ui->agentEdit->clear();
        this->ui->agentEdit->setText(outAgent);

    }
}

























