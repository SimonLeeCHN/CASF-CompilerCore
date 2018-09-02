#include "CompilerCommon.h"
#include "CompilerCore.h"
#include <QStringList>
#include <QDebug>

#define RFID_BASE_DEV           100

QString STA_Translate()
{
    return "STA";
}

QString END_Translate()
{
    return "END";
}

QString DEY_Translate(int msc)
{
    QString str = "DEY " + QString::number(msc);
    return str;
}

QString MOV_Translate(QList<int>&carrierPos,int leader,int num,int leaderGoal)
{
    QString str = QString("MOV %1 %2 %3").arg(QString::number(leader),QString::number(num),QString::number(leaderGoal));

    //修改carrierPos
    for(int i = 0;i < num;i++)
    {
        carrierPos[(leader - 1) + i] = leaderGoal + i*RFID_BASE_DEV;
    }

    return str;
}

QString CMG_Translate(int num)
{
    QString str = QString("CMG %1").arg(QString::number(num));
    return str;
}


QString BaseInstructTranslate(QStringList src,QList<int> &carrierPos,int rfidNum)
{
    int instPort = (src.at(0)).toInt();
    QString str;
    switch (instPort)
    {
        case INSTRUCT_BASE_STA:
            str = STA_Translate();
            break;
        case INSTRUCT_BASE_END:
            str = END_Translate();
            break;
        case INSTRUCT_BASE_DEY:
            if(src.count() == 2)
            {
                int msc = (src.at(1)).toInt();
                str = DEY_Translate(msc);
            }
            else
                str = "";
            break;
        case INSTRUCT_BASE_MOV:
            if(src.count() == 4)
            {
                int leader = (src.at(1)).toInt();
                int num = (src.at(2)).toInt();
                int leaderGoal = (src.at(3)).toInt();

                if(!(leaderGoal/RFID_BASE_DEV + num > rfidNum) && (leaderGoal > 0))
                {
                    str = MOV_Translate(carrierPos,leader,num,leaderGoal);
                }
                else
                {
                    //队头或尾出界
                    str = "";
                }
            }
            else
                str = "";
            break;
        case INSTRUCT_BASE_CMG:
            if(src.count() == 2)
            {
                int num = (src.at(1)).toInt();
                str = CMG_Translate(num);
            }
            else
                str = "";
            break;
        default:
            str = "";
            break;
    }
    return str;
}


/***************************************************************/


QList<QString> SOD_Translate(QList<int> &carrierPos,int rfidNum,int space,int align)
{
    QList<QString> outList;
    outList.clear();
    if(space < 0)
    {
        qDebug()<<"space不得小于0";
        return outList;
    }
    if(!IS_ALIGN_INSTRUCT(align))
    {
        qDebug()<<"align非标";
        return outList;
    }
    int carrierNum = carrierPos.count();
    if((carrierNum + (carrierNum-1)*space) > rfidNum)
    {
        qDebug()<<"超长";
        return outList;
    }

    switch (align)
    {
        case INSTRUCT_ALIGN_HEAD:
        {
            if((carrierNum + (carrierNum-1)*space) > (rfidNum - int(carrierPos.at(0)/RFID_BASE_DEV)))
            {
                qDebug()<<"头对齐间隔分布超长";
                return outList;
            }

            outList.append(CMG_Translate(carrierNum));

            //设置头载体车
            QString str = MOV_Translate(carrierPos,1,1,carrierPos.at(0));
            outList.append(str);
            str.clear();

            //推演后续车辆
            for(int index = 1;index < carrierNum;index++)
            {
                //后续车辆位为前车位加间距
                int goal = carrierPos.at(index - 1) + ((space + 1) * RFID_BASE_DEV);
                str = MOV_Translate(carrierPos,index + 1,1,goal);
                outList.append(str);
                str.clear();
            }

            break;
        }
        case INSTRUCT_ALIGN_MID:
        {
            int middleCarrier = int(double(carrierNum)/2.0 + 0.5);

            //以中间车辆的位置为基础，向前推演出头车目标，再推演后续车辆

            //头车目标 = 中间车辆点位 - 中间车辆左车数 - 中间车辆左总dummy数
            int leaderGoal = int(carrierPos.at(middleCarrier-1)/RFID_BASE_DEV) - (middleCarrier-1) - (middleCarrier-1)*space;

            if((carrierNum + (carrierNum-1)*space) > (rfidNum - leaderGoal))
            {
                qDebug()<<"居中对齐间隔分布超长";
                return outList;
            }

            outList.append(CMG_Translate(carrierNum));

            //设置头载体车
            QString str = MOV_Translate(carrierPos,1,1,leaderGoal*RFID_BASE_DEV);
            outList.append(str);
            str.clear();

            //推演后续车辆
            for(int index = 1;index < carrierNum;index++)
            {
                //后续车辆位为前车位加间距
                int goal = carrierPos.at(index - 1) + ((space + 1) * RFID_BASE_DEV);
                str = MOV_Translate(carrierPos,index + 1,1,goal);
                outList.append(str);
                str.clear();
            }

            break;
        }
        case INSTRUCT_ALIGN_TAIL:
        {
            if((carrierNum + (carrierNum-1)*space) > (int(carrierPos.last()/RFID_BASE_DEV)))
            {
                qDebug()<<"尾对齐间隔分布超长";
                return outList;
            }

            outList.append(CMG_Translate(carrierNum));

            //设置尾载体车
            QString str = MOV_Translate(carrierPos,carrierNum,1,carrierPos.at(carrierNum-1));
            outList.append(str);
            str.clear();

            //推演前列车辆
            for(int index = (carrierNum - 2);index >= 0;index--)
            {
                //前车位为后车位加间距
                int goal = carrierPos.at(index + 1) - ((space+1) * RFID_BASE_DEV);
                str = MOV_Translate(carrierPos,index + 1,1,goal);
                outList.append(str);
                str.clear();
            }

            break;
        }
        default:
            break;
    }

    return outList;

}

QList<QString> PUD_Translate(QList<int> &carrierPos,int rfidNum)
{
    //计算最大间隔
    int maxSpace = int((rfidNum - carrierPos.count())/(carrierPos.count() - 1));
    carrierPos[0] = 1 * RFID_BASE_DEV;
    QList<QString> outList = SOD_Translate(carrierPos,rfidNum,maxSpace,INSTRUCT_ALIGN_HEAD);
    return outList;
}

QList<QString> GAT_Translate(QList<int> &carrierPos,int align)
{
    QList<QString> outList;
    outList.clear();
    if(!IS_ALIGN_INSTRUCT(align))
    {
        qDebug()<<"align非标";
        return outList;
    }

    switch (align)
    {
        case INSTRUCT_ALIGN_HEAD:
        {
            outList.append(MOV_Translate(carrierPos,1,carrierPos.count(),carrierPos.at(0)));

            break;
        }
        case INSTRUCT_ALIGN_MID:
        {
            int middleCarrier = int(double(carrierPos.count())/2.0 + 0.5);
            int leaderGoal = carrierPos.at(middleCarrier-1) - (middleCarrier - 1)* RFID_BASE_DEV;

            outList.append(MOV_Translate(carrierPos,1,carrierPos.count(),leaderGoal));

            break;
        }
        case INSTRUCT_ALIGN_TAIL:
        {
            int leaderSpace = int(carrierPos.last()/RFID_BASE_DEV) - carrierPos.count();
            outList.append(MOV_Translate(carrierPos,1,carrierPos.count(),(leaderSpace + 1) * RFID_BASE_DEV));
        }
        default:
            break;
    }

    return outList;
}

QList<QString> EXP_Translate(QList<int> &carrierPos,int rfidNum,int rate,int align)
{
    QList<QString> outList;
    outList.clear();

    int dummyLen = int(((1*rate + ((carrierPos.count()-1)*rate)) * (carrierPos.count()-1))/2);
    int carreirGroupLen = dummyLen + carrierPos.count();
    if((carreirGroupLen) > rfidNum)
    {
        qDebug()<<"超长";
        return outList;
    }

    switch (align)
    {
        case INSTRUCT_ALIGN_HEAD:
        {
            if((carreirGroupLen) > (rfidNum - int(carrierPos.at(0)/RFID_BASE_DEV)))
            {
                qDebug()<<"膨胀头对齐超长";
                return outList;
            }

            outList.append(CMG_Translate(carrierPos.count()));

            //设置头载体车
            QString str = MOV_Translate(carrierPos,1,1,carrierPos.at(0));
            outList.append(str);
            str.clear();

            //推演后续车辆
            for(int index = 1;index < carrierPos.count();index++)
            {
                //后续车辆等差数量间距
                int goal = carrierPos.at(index-1) + (index * rate + 1) * RFID_BASE_DEV;
                str = MOV_Translate(carrierPos,index+1,1,goal);
                outList.append(str);
                str.clear();
            }

            break;
        }
        case INSTRUCT_ALIGN_MID:
        {

            break;
        }
        case INSTRUCT_ALIGN_TAIL:
        {
            if((carreirGroupLen) > (int(carrierPos.last()/RFID_BASE_DEV)))
            {
                qDebug()<<"膨胀尾对齐超长";
                return outList;
            }

            outList.append(CMG_Translate(carrierPos.count()));

            //设置尾载体车
            QString str = MOV_Translate(carrierPos,carrierPos.count(),1,carrierPos.last());
            outList.append(str);
            str.clear();

            //推演前列车辆
            for(int index = (carrierPos.count() - 2);index >= 0;index--)
            {
                int goal = carrierPos.at(index+1) - ((carrierPos.count() - index - 1) * rate + 1) * RFID_BASE_DEV;
                str = MOV_Translate(carrierPos,index+1,1,goal);
                outList.append(str);
                str.clear();
            }

            break;
        }
        default:
            break;
    }

    return outList;
}




QList<QString> AdvanceInstructTranslate(QStringList src,QList<int> &carrierPos,int rfidNum)
{
    int instPort = (src.at(0)).toInt();
    QList<QString> outList;
    outList.clear();

    switch (instPort)
    {
        case INSTRUCT_ADVANCE_SOD:
        {
            if(src.count() == 3)
            {
                int space = (src.at(1)).toInt();
                int align = (src.at(2)).toInt();
                outList.append(SOD_Translate(carrierPos,rfidNum,space,align));
            }

            break;
        }
        case INSTRUCT_ADVANCE_PUD:
        {
            outList.append(PUD_Translate(carrierPos,rfidNum));
            break;
        }
        case INSTRUCT_ADVANCE_GAT:
        {
            if(src.count() == 2)
            {
                int align = (src.at(1)).toInt();
                outList.append(GAT_Translate(carrierPos,align));
            }

            break;
        }
        case INSTRUCT_ADVANCE_EXP:
        {
            if(src.count() == 3)
            {
                int rate = (src.at(1)).toInt();
                int align = (src.at(2)).toInt();
                outList.append(EXP_Translate(carrierPos,rfidNum,rate,align));
            }
            break;
        }
        default:
            break;
    }
    return outList;
}


/***************************************************************/




/*
 *  src--传入指令集
 *  out--转换为casf文件指令集
 */
int InstructTranslate(QList<QString> src, QList<QString> &out, QList<int> &carrierPos, int rfidNum)
{
    out.clear();
    int status = 0;
    int instNum = src.count();

    for(int instIndex = 0;instIndex < instNum;instIndex++)
    {
        QString lineInstruct = src.at(instIndex);
        QStringList lineInstParm =lineInstruct.split(" ");

        //取得命令端口
        int instPort = (lineInstParm.at(0)).toInt();

        //判别命令归属
        if(IS_BASE_INSTRUCT(instPort))
        {
            out.append(BaseInstructTranslate(lineInstParm,carrierPos,rfidNum));
            status++;
        }
        else if(IS_ADVANCE_INSTRUCT(instPort))
        {
            out.append(AdvanceInstructTranslate(lineInstParm,carrierPos,rfidNum));
            status++;
        }

    }
    return status;
}
