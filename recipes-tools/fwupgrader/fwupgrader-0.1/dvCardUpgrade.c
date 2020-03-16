
bool goToBootloader(DWORD dwCompany)
{
    bool ret = false;

    changeBaudrate(m_strCompanyCfg.at(GET_COMPANY_BAUDRATE(static_cast<int>(dwCompany))).toInt());

    m_transferProtocol->m_serial->m_port->clear();
    m_transferProtocol->m_serial->write_data(m_strCompanyCfg.at(GET_COMPANY_BOOTCLI(static_cast<int>(dwCompany))).toUtf8());

    this->thread()->sleep(2);

    changeBaudrate(BOOTCODE_BAUDRATE);
    m_transferProtocol->m_serial->m_port->clear();

    return ret;
}

bool setIapEnable(DWORD dwStartAddress, DWORD dwBinSize)
{
    bool ret = false;
    BYTE cDummy = 0;
    sIAP_INFO sInfo;

    sInfo.dwStart = dwStartAddress;
    sInfo.dwSize = dwBinSize;

    if(false == commandWrite(eIAP_CMD_APP_INFO, sizeof(sIAP_INFO), reinterpret_cast<BYTE *>(&sInfo)))
        return false;

    ret = commandWrite(eIAP_CMD_IAP_ENABLE, 0, &cDummy);

    return ret;
}

bool writePageBinary(WORD wSize, BYTE *pcBuffer)
{
    bool ret = false;

    ret = commandWrite(eIAP_CMD_BIN_DATA, wSize, pcBuffer);

    return ret;
}

bool checkPageChecksum(DWORD dwChecksum)
{
    bool ret = false;
    QByteArray retChecksum;
    DWORD *pdwRetChecksum = nullptr;

    ret = commandRead(eIAP_CMD_BIN_CHECK_SUM, retChecksum);

    pdwRetChecksum = reinterpret_cast<DWORD *>(retChecksum.data());

    if(0 == ((*pdwRetChecksum + dwChecksum) & 0xFFFFFFFF))
    {
        ret = true;
    }

    return ret;
}

bool programPage(DWORD dwAddress)
{
    bool ret = false;

    ret = commandWrite(eIAP_CMD_PROGRAMING, sizeof(DWORD), reinterpret_cast<BYTE*>(&dwAddress));

    return ret;
}

bool writeTag(DWORD dwFwVersion, DWORD dwCompany)
{
    bool ret = false;
    DWORD dwTag[2];
    dwTag[0] = dwFwVersion;
    dwTag[1] = dwCompany;

    ret = commandWrite(eIAP_CMD_PROGRAMING_FINISH, sizeof(DWORD)*2, reinterpret_cast<BYTE*>(dwTag));
    return ret;
}

bool goToAppCode()
{
    bool ret = false;
    ret = commandWrite(eIAP_CMD_RUN_APP, 0, nullptr);
    return ret;
}

void loadFile(const QString &fileName)
{
    sMEM_TAG_PARAM *psTag = nullptr;
    quint32 address = 0;
    char *data = nullptr;
    QString string;
    int count = 0;

    m_intelToBin->open(fileName, (1024));
    m_intelToBin->reReadAll();
    m_intelToBin->selectSegment(1);
    m_intelToBin->readPage(address, &data, 0);

    psTag = reinterpret_cast<sMEM_TAG_PARAM*>(data);

    m_dwFwVersion = psTag->dwCodeVersion;

    for(count = 0; count < GET_COMPANY_SIZE; count++)
    {
        if(psTag->dwCompany == static_cast<DWORD>(m_strCompanyCfg.at(GET_COMPANY_ID(count)).toInt()))
        {
            m_dwCompany = static_cast<DWORD>(count);
            break;
        }
    }

    if(GET_COMPANY_SIZE > count){
        string = tr("%1 V%2.%3").arg(m_strCompanyCfg.at(GET_COMPANY_NAME(count))).arg((m_dwFwVersion >> 8 )&0x00FF).arg(QString::number(m_dwFwVersion&0x00FF));
        showString(string);
    }
    else {
        popMessage(tr("Firmware file is incorrect!!\r\nPlease try again!!"));
    }
}

bool upgrade(void)
{
    WORD wTotalPages = 0;
    DWORD dwChecksum = 0;
    uint32 address = 0;
    char *data = null;

    //enableButtons(false);
    //showString("Starting please wait...");

    m_intelToBin->reReadAll();
    m_intelToBin->selectSegment(1);

    // 1. go to bootloader
    goToBootloader(m_dwCompany);

    // 2. Enable In Application Programming
    if(false == setIapEnable(APP_START_ADDRESS, m_intelToBin->segmentSize())) {
        //popMessage(tr("Can't enter programming mode!\r\nPlease check the connection and try again!!"));
        goto ERROR;
    }

    // 3. write binary data and check the checksum
    while (0 != m_intelToBin->readPage(address, &data, 0)) {
        //qDebug() << QString::number(address, 16);
        dwChecksum = 0;

        for (uint32_t i = 0; i < m_intelToBin->pageSize; i++) {
            dwChecksum += data[i];//static_cast<BYTE>(data[i]);
        }

        if(false == writePageBinary(static_cast<WORD>(m_intelToBin->pageSize), reinterpret_cast<BYTE *>(data))) {
            //popMessage(tr("Transfer data Error!\r\nPlease try again!!"));
            goto ERROR;
        }

        if(true == checkPageChecksum(dwChecksum)) {
            programPage(address);
            //updateProgress(0, (m_intelToBin->segmentPages()-1), wTotalPages++);
        }
        else {
            //popMessage(tr("Checksum Error!\r\nPlease try again!!"));
            goto ERROR;
        }
    }

    // 4. write tag
    if(false == writeTag(m_dwFwVersion, m_dwCompany)){
        //popMessage(tr("Transfer data Error!\r\nPlease try again!!"));
        goto ERROR;
    }

    // 5. go to appcode
    goToAppCode();
    //enableButtons(true);
    return;

ERROR:
    //enableButtons(true);
    //showString("Error! Please try again.");
}

