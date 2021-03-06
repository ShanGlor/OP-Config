#include "tablemodel_functiontriggers.h"

// What is this QModelIndex business everywhere?
// Every QAbstract model is conceptualized by three dimensions: row, column, and hierarchy.
// A list model will only have rows, and column will always be 0 (1 column)
// A table model will have rows and columns, but no hierarchy
// A tree model will have rows and columns, and the third dimension represents the Z-order if you will of the data in question
// For models that don't use one of these dimensions, you can just type in QModelIndex() instead, because something is still required.

FunctionTriggerTableModel::FunctionTriggerTableModel(QObject *parent) : QAbstractTableModel(parent)
{
    OPQMap = new OP_QMaps;                             // Used to get special function descriptions - this is not part of the table model
    SFQMap = OPQMap->getAllSpecialFunctionsQMap();     // Local copy of the special functions QMap
    TSQMap = OPQMap->getAllTriggerSourcesQMap();       // Local copy of the trigger sources QMap
    TurretPositionsQMap = OPQMap->getTurretStickSpecialPositionsQMap();  // Turret stick positions QMap
}

// This constructor allows you to pass data to populate the model at the beginning
FunctionTriggerTableModel::FunctionTriggerTableModel(QList< functionTriggerData > ftdata, QObject *parent) : QAbstractTableModel(parent)
{
    OPQMap = new OP_QMaps;                             // Used to get special function descriptions - this is not part of the table model
    SFQMap = OPQMap->getAllSpecialFunctionsQMap();     // Local copy of the special functions QMap
    TSQMap = OPQMap->getAllTriggerSourcesQMap();       // Local copy of the trigger sources QMap
    TurretPositionsQMap = OPQMap->getTurretStickSpecialPositionsQMap();  // Turret stick positions QMap

    listOfTriggerData = ftdata;
}

void FunctionTriggerTableModel::CustomChangeSlot()
{   // Emit this any time data is changed
    emit TurretStickPresent(isTurretStickPresent());
    // Emit the row count
    emit functionTriggerCount(this->rowCount());
}

int FunctionTriggerTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);   // Turn off compiler warnings about unreferenced parameters
    return listOfTriggerData.size();
}

int FunctionTriggerTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);   // Turn off compiler warnings about unreferenced parameters
    return 4;      // We have a constant 4 columns, equal to the 4 members of the functionTriggerData members
}

// The data() function returns one of the four members of our struct (turned into columns), based on the contents of the model index supplied.
// The row number stored in the model index is used to reference an item in the list of data.
QVariant FunctionTriggerTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())    return QVariant();

    if (index.row() >= listOfTriggerData.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        functionTriggerData ftd = listOfTriggerData.at(index.row());

        if (index.column() == 0)
            return ftd.functionTrigger.TriggerID;
        else if (index.column() == 1)
            return ftd.functionTrigger.specialFunction;
        else if (index.column() == 2)
            return ftd.FunctionDescription;
        else if (index.column() == 3)
            return ftd.TriggerDescription;
    }
    return QVariant();
}

// The headerData() function displays the table's headers, in our case, TriggerID, SpecialFunction, TriggerDescription, and FunctionDescription.
// If you require numbered entries for your address book, you can use a vertical header (see the AddressWidget implementation).
QVariant FunctionTriggerTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case 0:
                return tr("TriggerID");
                break;

            case 1:
                return tr("FunctionID");
                break;

            case 2:
                return tr("Function");
                break;

            case 3:
                return tr("Trigger");
                break;

            default:
                return QVariant();
        }
    }
    else
    {   // Row numbers along the left
        //return QString("Row %1").arg(section);
        return QString::number(section + 1);    // add 1 because it's zero based
    }

    return QVariant();
}


// The insertRows() function is called before new data is added, otherwise the data will not be displayed.
// The beginInsertRows() and endInsertRows() functions are called to ensure all connected views are aware of the changes.
bool FunctionTriggerTableModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);
    for (int row=0; row < rows; row++)
    {
        // Blank row
        functionTriggerData ftd;
        ftd.functionTrigger.TriggerID = 0;
        ftd.functionTrigger.specialFunction = SF_NULL_FUNCTION;
        ftd.FunctionDescription = "";
        ftd.TriggerDescription = "";

        listOfTriggerData.insert(position, ftd);
    }
    endInsertRows();

    // Any time data is changed, we emit a few custom signals
    CustomChangeSlot();

    return true;
}

// This just calls insertRows() with one row
bool FunctionTriggerTableModel::insertRow(int position, const QModelIndex &index)
{
    return this->insertRows(position, 1, index);
}

// The removeRows() function is called to remove data. Again, beginRemoveRows() and endRemoveRows() are called
// to ensure all connected views are aware of the changes.
bool FunctionTriggerTableModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position+rows-1);
    for (int row=0; row < rows; ++row)
    {
        listOfTriggerData.removeAt(position);
    }
    endRemoveRows();

    // Any time data is changed, we emit a few custom signals
    CustomChangeSlot();

    return true;
}

// This just calls removeRows() with one row
bool FunctionTriggerTableModel::removeRow(int position, const QModelIndex &index)
{
    return this->removeRows(position, 1, index);
}

// This removes every row from the model
bool FunctionTriggerTableModel::ClearModel(void)
{
    return this->removeRows(0, this->rowCount());
}

// The setData() function is the function that inserts data into the table, item by item and not row by row.
// This means that to fill a row in the address book, setData() must be called four times for this particular model,
// as each row has 4 columns. It is important to emit the dataChanged() signal as it tells all connected views to update their displays.
bool FunctionTriggerTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        int row = index.row();

        functionTriggerData ftd = listOfTriggerData.value(row);

        if (index.column() == 0)
            ftd.functionTrigger.TriggerID = value.toUInt();
        else if (index.column() == 1)
            ftd.functionTrigger.specialFunction = static_cast<_special_function>(value.toUInt());
        else if (index.column() == 2)
            ftd.FunctionDescription = value.toString();
        else if (index.column() == 3)
            ftd.TriggerDescription = value.toString();
        else
            return false;

        listOfTriggerData.replace(row, ftd);
        emit(dataChanged(index, index));

        // Any time data is changed, we emit a few custom signals
        CustomChangeSlot();

        return true;
    }
    else
    {
        return false;
    }
}

// The flags() function returns the item flags for the given index.
Qt::ItemFlags FunctionTriggerTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index);
    // If you wanted to be able to directly edit the data in the table model, you can set
    // the Qt::ItemIsEditable flag like so. However even without it set, we are still able to
    // add/remove rows programmatically.
    //return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}


// Custom Functions
// The last function in TableModel, getList() returns the data object that holds all the data.
QList< functionTriggerData > FunctionTriggerTableModel::getList()
{
    return listOfTriggerData;
}

bool FunctionTriggerTableModel::insertFunctionTrigger(_special_function sf, uint16_t trID)
{
    // Here we pass a _special_function number and a TriggerID, and a row is added to the table model.
    // The special function description and the trigger ID description are auto generated
    functionTriggerData ftd;
    ftd.functionTrigger.TriggerID = trID;
    ftd.functionTrigger.specialFunction = sf;
    ftd.FunctionDescription = SFQMap.value(sf);
    ftd.TriggerDescription = getTriggerDescription(sf, trID);

    // First - are we trying to add more function-triggers than allowed?
    int numRows = this->rowCount(QModelIndex());
    if (numRows >= MAX_FUNCTION_TRIGGERS)
    {
        this->setErrorText(QString("You have reached the max number of function triggers (%1).\n\nYou must remove one before you can add more.").arg(MAX_FUNCTION_TRIGGERS));
        return false;
    }

    // Second - does this function trigger already exist?
    for (int i = 0; i < numRows ; i++)
    {
        if ((trID == this->data(this->index(i,0), Qt::DisplayRole)) && (sf == this->data(this->index(i,1), Qt::DisplayRole)))
        {
            this->setErrorText("That function-trigger combination already exists!");
            return false;
        }
    }

    // Ok, we haven't reached the max, and it doesn't already exist. Add it.
    QModelIndex index;
    this->insertRow(numRows);   // Insert a new row at the end (numRows will be the last one)
    index = this->index(numRows, 0, QModelIndex());
    this->setData(index, ftd.functionTrigger.TriggerID, Qt::EditRole);
    index = this->index(numRows, 1, QModelIndex());
    this->setData(index, ftd.functionTrigger.specialFunction, Qt::EditRole);
    index = this->index(numRows, 2, QModelIndex());
    this->setData(index, ftd.FunctionDescription, Qt::EditRole);
    index = this->index(numRows, 3, QModelIndex());
    this->setData(index, ftd.TriggerDescription, Qt::EditRole);

    // Any time data is changed, we emit a few custom signals
    CustomChangeSlot();

    return true;

}

boolean FunctionTriggerTableModel::isTurretStickPresent(void)
{   // Are there any triggers in the model that belong to the turret stick: return true/false
    int TriggerID;
    int numRows = this->rowCount(QModelIndex());
    for (int i = 0; i < numRows ; i++)
    {
        TriggerID = TriggerIDAtRow(i);
        if (TriggerID > 0 && TriggerID <= MAX_SPEC_POS)
            return true;
    }
    return false;
}

QString FunctionTriggerTableModel::getTriggerDescription(_special_function sf, uint16_t TriggerID)
{
    QString TriggerDescription = "";
    int TriggerAction = 0;

    // Is the selected function digital or analog:
    boolean sf_digital = OPQMap->isSpecialFunctionDigital(sf);

    // Get the trigger source and action:
    _trigger_source ts = TS_NULL_TRIGGER; // If we don't initalize to something we get annoying compiler warnings

    if (TriggerID > 0 && TriggerID <= MAX_SPEC_POS)
    {   // Turret stick trigger source
        ts = TS_TURRET_STICK;
        // Trigger action in this case is equal to the Trigger ID
        TriggerDescription = "Turret Stick - ";
        TriggerDescription.append(TurretPositionsQMap.value(static_cast<turretStick_positions>(TriggerID)));
    }
    else if (TriggerID >= trigger_id_multiplier_ports && TriggerID < trigger_id_multiplier_auxchannel)
    {
        // External I/O source. The Trigger Source is the integer value of the division by the ports multiplier (discard remainder)
        int portNum = TriggerID / trigger_id_multiplier_ports;
        switch (portNum)
        {
            case 1:
                ts = TS_INPUT_A;
                break;
            case 2:
                ts = TS_INPUT_B;
                break;
        }
        // This gives us something like "External Input A"
        TriggerDescription = TSQMap.value(ts);
        TriggerDescription.append(" - ");
        if (sf_digital)
        {
            // The remainder is the action (on/off if digital, or variable if analog)
            TriggerAction = getTriggerActionFromTriggerID(TriggerID);
            if (TriggerAction == 0) TriggerDescription.append("Off");
            if (TriggerAction == 1) TriggerDescription.append("On");
        }
        else
        {
            TriggerDescription.append("Variable");
        }
    }
    else if (TriggerID >= trigger_id_multiplier_auxchannel && TriggerID < trigger_id_adhoc_start)
    {
        // Aux RC channel inputs
        int channelNum = TriggerID / trigger_id_multiplier_auxchannel;
        switch (channelNum)
        {
            case 1: ts = TS_AUX1;   break;
            case 2: ts = TS_AUX2;   break;
            case 3: ts = TS_AUX3;   break;
            case 4: ts = TS_AUX4;   break;
            case 5: ts = TS_AUX5;   break;
            case 6: ts = TS_AUX6;   break;
            case 7: ts = TS_AUX7;   break;
            case 8: ts = TS_AUX8;   break;
            case 9: ts = TS_AUX9;   break;
            case 10: ts = TS_AUX10;   break;
            case 11: ts = TS_AUX11;   break;
            case 12: ts = TS_AUX12;   break;
        }
        // This gives us something like "Aux Channel 1"
        TriggerDescription = TSQMap.value(ts);
        TriggerDescription.append(" - ");
        if (sf_digital)
        {
            // But we still need to append the position.
            TriggerAction = getTriggerActionFromTriggerID(TriggerID);
            int numPositions = getNumPositionsFromTriggerID(TriggerID);
            TriggerDescription.append(QString("Pos %1 (of %2)").arg(TriggerAction).arg(numPositions));
        }
        else
        {
            TriggerDescription.append("Variable");
        }
    }
    // Ad-Hoc Triggers
    else if (TriggerID >= trigger_id_adhoc_start && TriggerID < (trigger_id_adhoc_start + trigger_id_adhoc_range))
    {   // The only way to do these is hand-code them, that is why they are called "ad-hoc"
        switch (TriggerID)
        {
            case ADHOC_TRIGGER_BRAKES_APPLIED:
                ts = TS_ADHC_BRAKES;
                TriggerDescription = "Brakes Applied";
                break;
            case ADHOC_TRIGGER_CANNON_HIT:
                ts = TS_ADHC_CANNONHIT;
                TriggerDescription = "Cannon Hit";
                break;
            case ADHOC_TRIGGER_VEHICLE_DESTROYED:
                ts = TS_ADHC_DESTROYED;
                TriggerDescription = "Vehicle Destroyed";
                break;
            case ADHOC_TRIGGER_CANNON_RELOADED:
                ts = TS_ADHC_CANNONRELOAD;
                TriggerDescription = "Cannon Reloaded";
                break;
        }
    }
    // Vehicle speed triggers - increasing speed
    else if (TriggerID >= trigger_id_speed_increase && TriggerID < (trigger_id_speed_increase + trigger_id_speed_range))
    {
        uint8_t triggerSpeed = TriggerID - trigger_id_speed_increase;  // The remainder is the percent we want to check against
        // This also works
        // TriggerAction = getTriggerActionFromTriggerID(TriggerID);
        ts = TS_SPEED_INCR;
        TriggerDescription = "Vehicle Speed Increases Above ";
        TriggerDescription.append(QString("%1\%").arg(triggerSpeed));
    }
    // Vehicle speed triggers - decreasing speed
    else if (TriggerID >= trigger_id_speed_decrease && TriggerID < (trigger_id_speed_decrease + trigger_id_speed_range))
    {
        uint8_t triggerSpeed = TriggerID - trigger_id_speed_decrease;  // The remainder is the percent we want to check against
        ts = TS_SPEED_DECR;
        TriggerDescription = "Vehicle Speed Decreases Below ";
        TriggerDescription.append(QString("%1\%").arg(triggerSpeed));
    }
    // Throttle command - variable (analog) trigger
    else if (TriggerID == trigger_id_throttle_command)
    {
        ts = TS_THROTTLE_COMMAND;
        TriggerDescription = "Throttle Command";
    }
    // Engine speed - variable (analog) trigger
    else if (TriggerID == trigger_id_engine_speed)
    {
        ts = TS_ENGINE_SPEED;
        TriggerDescription = "Engine Speed";
    }
    // Vehicle speed - variable (analog) trigger
    else if (TriggerID == trigger_id_vehicle_speed)
    {
        ts = TS_VEHICLE_SPEED;
        TriggerDescription = "Vehicle Speed";
    }
    // Steering command - variable (analog) trigger
    else if (TriggerID == trigger_id_steering_command)
    {
        ts = TS_STEERING_COMMAND;
        TriggerDescription = "Steering Command";
    }
    // Turret rotation command - variable (analog) trigger
    else if (TriggerID == trigger_id_rotation_command)
    {
        ts = TS_ROTATION_COMMAND;
        TriggerDescription = "Turret Rotation Command";
    }
    // Barrel elevation command - variable (analog) trigger
    else if (TriggerID == trigger_id_elevation_command)
    {
        ts = TS_ELEVATION_COMMAND;
        TriggerDescription = "Barrel Elevation Command";
    }

    return TriggerDescription;
}

uint8_t FunctionTriggerTableModel::getNumPositionsFromTriggerID(uint16_t TriggerID)
{
    // Turret stick trigger - num positions is not relevant
    if (TriggerID >0 && TriggerID <= MAX_SPEC_POS)
        return 0;

    // External input trigger - if digital, these are only 2-position switches:
    // On (line connected to ground) or Off (line disconnected/held high by internal pullups)
    if (TriggerID >= trigger_id_multiplier_ports && TriggerID < trigger_id_multiplier_auxchannel)
        return 2;

    // Aux Channel switches - if digital, these can be 2-6 position switches
    if (TriggerID >= trigger_id_multiplier_auxchannel)
    {
        // We will walk through an example as we calculate these values.
        // Assume the Trigger ID is 4063
        // channelNum will equal 4 (Aux Channel 4)
        int channelNum = TriggerID / trigger_id_multiplier_auxchannel;
        // NumPos_CurPos will equal 63. 6 is the number of positions the switch has, 3 is the trigger position number
        int NumPos_CurPos = TriggerID - (channelNum * trigger_id_multiplier_auxchannel);
        // NumPositions will equal 6. This is the number of positions the switch is capable of
        int NumPositions = NumPos_CurPos / switch_pos_multiplier;   // Drop the remainder
        return NumPositions;
        // Trigger Action will equal 3, we don't need it in this function though, see the next function getTriggerActionFromTriggerID
        // int TriggerAction = NumPos_CurPos - (NumPositions * switch_pos_multiplier);
    }

    // Any other case, return 0
        return 0;
}

uint8_t FunctionTriggerTableModel::getTriggerActionFromTriggerID(uint16_t TriggerID)
{   // Remember also, for analog triggers, there is no "action"

    int TriggerAction = 0;

    // Turret stick trigger - action *is* the trigger ID
    if (TriggerID >0 && TriggerID <= MAX_SPEC_POS)
        return TriggerID;   // should auto-cast to uint8_t. Anyway it doesn't matter, we won't be looking up TriggerActions for turret stick

    // External input trigger - if digital, these are only 2-position switches:
    // On (line connected to ground) or Off (line disconnected/held high by internal pullups)
    if (TriggerID >= trigger_id_multiplier_ports && TriggerID < trigger_id_multiplier_auxchannel)
    {
        int portNum = TriggerID / trigger_id_multiplier_ports;

        // The remainder is the action (1/0)
        TriggerAction = TriggerID - (portNum * trigger_id_multiplier_ports);
        return TriggerAction;
    }

    // Aux Channel switches
    if (TriggerID >= trigger_id_multiplier_auxchannel && TriggerID < trigger_id_adhoc_start)
    {
        // We will walk through an example as we calculate these values.
        // Assume the Trigger ID is 4063
        // channelNum will equal 4 (Aux Channel 4)
        int channelNum = TriggerID / trigger_id_multiplier_auxchannel;
        // NumPos_CurPos will equal 63. 6 is the number of positions the switch has, 3 is the trigger action we want.
        int NumPos_CurPos = TriggerID - (channelNum * trigger_id_multiplier_auxchannel);
        // NumPositions will equal 6. This is the number of positions the switch is capable of
        int NumPositions = NumPos_CurPos / switch_pos_multiplier; // Drop remainder
        // Trigger Action will equal 3
        TriggerAction = NumPos_CurPos - (NumPositions * switch_pos_multiplier);
        return TriggerAction;
    }

    // Ad-Hoc - no actions
    if (TriggerID >= trigger_id_adhoc_start && TriggerID < (trigger_id_adhoc_start + trigger_id_adhoc_range))
        return 0;

    // Vehicle speed triggers - increasing speed
    if (TriggerID >= trigger_id_speed_increase && TriggerID < (trigger_id_speed_increase + trigger_id_speed_range))
        return (TriggerID - trigger_id_speed_increase);  // The remainder is the percent we want to check against, ie, the action

    // Vehicle speed triggers - decreasing speed
    else if (TriggerID >= trigger_id_speed_decrease && TriggerID < (trigger_id_speed_decrease + trigger_id_speed_range))
        return (TriggerID - trigger_id_speed_decrease);  // The remainder is the percent we want to check against, ie, the action

    // Any other case, return 0
    return 0;
}

uint8_t FunctionTriggerTableModel::getAuxChannelNumberFromTriggerID(uint16_t TriggerID)
{
    if (TriggerID >= trigger_id_multiplier_auxchannel)
    {
        return TriggerID / trigger_id_multiplier_auxchannel;
    }
    else
    {
        return 0;
    }
}

boolean FunctionTriggerTableModel::removeAuxTriggers(int AuxChannelNum)
{   // This function removes any trigger belonging to the aux channel number given
    // It is used to remove function-triggers whenever the user sets an aux channel to "N/A"

    uint16_t TriggerID;
    int AuxChannel;
    boolean removedAny = false;

    int numRows = this->rowCount(QModelIndex());
    for (int i = numRows-1; i>=0; i--)
    {
        TriggerID = TriggerIDAtRow(i);
        AuxChannel = getAuxChannelNumberFromTriggerID(TriggerID);
        if (AuxChannel == AuxChannelNum)
        {   // We need to remove this row
            this->removeRow(i);
            removedAny = true;
        }
    }

    return removedAny;
}

boolean FunctionTriggerTableModel::checkAuxChannelTypesAgainstFunctionTriggers(int AuxChannelNum, boolean isDigital)
{   // This function removes any trigger belonging to an aux channel if the aux channel type and
    // function trigger type don't match. In other words, we remove digital function triggers
    // assigned to aux channels set to analog, and vice versa

    _special_function sf;
    boolean isSFDigital;
    int AuxChannel;
    boolean removedAny = false;

    int numRows = this->rowCount(QModelIndex());
    for (int i = numRows-1; i>=0; i--)
    {
        sf = SpecialFunctionAtRow(i);
        isSFDigital = OPQMap->isSpecialFunctionDigital(sf);
        AuxChannel = getAuxChannelNumberFromTriggerID(TriggerIDAtRow(i));
        if (AuxChannel == AuxChannelNum && isSFDigital != isDigital)
        {   // We need to remove this row.
            this->removeRow(i);
            removedAny = true;
        }
    }

    return removedAny;
}

boolean FunctionTriggerTableModel::checkAuxChannelPositionsAgainstFunctionTriggers(int AuxChannelNum, int AuxNumPositions)
{   // This function removes any trigger belonging to an aux channel if the number of total positions in the
    // trigger no longer matches the number of positions of the aux channel

    uint16_t TriggerID;
    _special_function sf;
    boolean isSFDigital;
    int AuxChannel;
    int NumPositions;
    boolean removedAny = false;

    int numRows = this->rowCount(QModelIndex());
    for (int i = numRows-1; i>=0; i--)
    {
        TriggerID = TriggerIDAtRow(i);
        sf = SpecialFunctionAtRow(i);
        isSFDigital = OPQMap->isSpecialFunctionDigital(sf);
        AuxChannel = getAuxChannelNumberFromTriggerID(TriggerID);
        NumPositions = getNumPositionsFromTriggerID(TriggerID);

        // We don't need to worry about analog functions here
        if (isSFDigital)
        {
            if (AuxChannel == AuxChannelNum && NumPositions != AuxNumPositions)
            {   // We need to remove this row.
                this->removeRow(i);
                removedAny = true;
            }
        }
    }

    return removedAny;

}

boolean FunctionTriggerTableModel::checkExternalPortDirectionAgainstFunctionTriggers(int PortNum, boolean PortIsOutput)
{   // This function removes any trigger belonging to an external port if the port direction is not set to input
    // It also removes any external port output functions if the port direction is not set to output

    _special_function sf;
    int SF_PortNum;
    int Trigger_PortNum;
    int TriggerID;
    boolean removedAny = false;


    int numRows = this->rowCount(QModelIndex());
    //for (int i = 0; i < numRows ; i++)
    for (int i = numRows-1; i>=0; i--)
    {
        // First, check port triggers (which should be inputs) to make sure those ports are set to input
        TriggerID = TriggerIDAtRow(i);
        Trigger_PortNum = getExternalPortNumberFromTriggerID(TriggerID);
        if (Trigger_PortNum == PortNum && PortIsOutput)    // Should be input
        {   // We need to remove this row.
            this->removeRow(i);
            removedAny = true;
        }
        else    // We don't need to remove this row twice, so only check this second part if the first part did nothing
        {
            // Next, check any port functions (which are outputs) to make sure those ports are set to output
            sf = SpecialFunctionAtRow(i);
            SF_PortNum = OPQMap->GetSpecialFunctionExternalPortNum(sf);
            if (SF_PortNum == PortNum && !PortIsOutput) // should be output
            {
                // We need to remove this row.
                this->removeRow(i);
                removedAny = true;
            }
        }
    }
    return removedAny;
}

boolean FunctionTriggerTableModel::checkExternalPortInputTypeAgainstFunctionTriggers(int PortNum, boolean isDigital)
{   // This function removes any input trigger belonging to an external input port if the port type and
    // function trigger type don't match. In other words, we remove digital function triggers
    // assigned to external analog inputs, and we remove analog function triggers assigned to
    // external digital inputs

    _special_function sf;
    boolean isSFDigital;
    int Trigger_PortNum;
    int TriggerID;
    boolean removedAny = false;

    int numRows = this->rowCount(QModelIndex());
    for (int i = numRows-1; i>=0; i--)
    {
        sf = SpecialFunctionAtRow(i);
        isSFDigital = OPQMap->isSpecialFunctionDigital(sf);
        TriggerID = TriggerIDAtRow(i);
        Trigger_PortNum = getExternalPortNumberFromTriggerID(TriggerID);
        if (Trigger_PortNum == PortNum && isSFDigital != isDigital)
        {   // We need to remove this row.
            this->removeRow(i);
            removedAny = true;
        }
    }
    return removedAny;
}


boolean FunctionTriggerTableModel::removeFunctionFromList(_special_function sf)
{   // Pass a special function and if it exists in the list, it will be removed.
    // If anything is removed, the function will return true, if not, false.
    boolean removedAny = false;

    int numRows = this->rowCount(QModelIndex());
    for (int i = numRows-1; i>=0; i--)
    {
        if (sf == SpecialFunctionAtRow(i))
        {   // Remove this row.
            this->removeRow(i);
            removedAny = true;
        }
    }
    return removedAny;
}

uint8_t FunctionTriggerTableModel::getExternalPortNumberFromTriggerID(uint16_t TriggerID)
{
    if (TriggerID >= trigger_id_multiplier_ports && TriggerID < trigger_id_multiplier_auxchannel)
    {
        return TriggerID / trigger_id_multiplier_ports;
    }
    else
    {
        return 0;
    }
}

uint16_t FunctionTriggerTableModel::TriggerIDAtRow(int row)
{
    // Trigger ID is column 0
    return this->data(this->index(row,0), Qt::DisplayRole).toUInt();
}

_special_function FunctionTriggerTableModel::SpecialFunctionAtRow(int row)
{
    // Special function is column 1
    return static_cast<_special_function>(this->data(this->index(row,1), Qt::DisplayRole).toUInt());
}

QString FunctionTriggerTableModel::errorText()
{
    return _error_text;
}

void FunctionTriggerTableModel::setErrorText(QString err_text)
{
    _error_text = err_text;
}
