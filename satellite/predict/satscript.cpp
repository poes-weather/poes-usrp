/*
    HRPT-Decoder, a software for processing POES high resolution weather satellite imagery.
    Copyright (C) 2010 Free Software Foundation, Inc.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Email: <postmaster@poes-weather.com>
    Web: <http://www.poes-weather.com>
*/

//---------------------------------------------------------------------------
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <stdlib.h>

#include "satscript.h"

//---------------------------------------------------------------------------
// these CAN NOT have embedded items
#define SS_INDEX_SATFREQ            0   // {SATFREQ}
#define SS_INDEX_SATNAME            1   // {SATNAME}

// these CAN have embedded items or parameters inside brackets
#define SS_INDEX_DATETIME           2   // {DATETIME:yyyy-MM-dd}
#define SS_INDEX_DIR                3   // {DIR:/path/to/directory}

#define SS_TMPSTR_SIZE              512

// commands
#define SS_CMD_OUT_FILE             0   // out-file
#define SS_CMD_BASEBAND_DIR         1   // baseband-dir
#define SS_CMD_BASEBAND_EXT         2   // baseband-file-ext
#define SS_CMD_FRAMES_DIR           3   // frames-dir
#define SS_CMD_FRAMES_EXT           4   // frames-file-ext

//---------------------------------------------------------------------------
TSatScript::TSatScript(void)
{
    _downlink = "0";

    _rx_script_args = new QStringList;
    _postproc_script_args = new QStringList;

    _flags = 0;

    constants = NULL;
    commands  = NULL;
    tmpstr = NULL;
}

//---------------------------------------------------------------------------
TSatScript::TSatScript(const TSatScript& src)
{
    _downlink = src.downlink();

    _rx_script = src.rx_script();
    _rx_script_args = new QStringList(src.rx_script_args());

    _postproc_script = src.postproc_script();
    _postproc_script_args = new QStringList(src.postproc_script_args());

    _flags = src.flags();

    constants = NULL;
    commands  = NULL;
    tmpstr = NULL;
}

//---------------------------------------------------------------------------
TSatScript::~TSatScript(void)
{
    delete _rx_script_args;
    delete _postproc_script_args;

    free_private();
}

//---------------------------------------------------------------------------
void TSatScript::free_private(void)
{
    if(constants)
        delete constants;
    if(commands)
        delete commands;
    if(tmpstr)
        free(tmpstr);

    constants = NULL;
    commands = NULL;
    tmpstr = NULL;
}

//---------------------------------------------------------------------------
TSatScript& TSatScript::operator = (const TSatScript& src)
{
    if(this == &src)
        return *this;

    _downlink = src.downlink();

    _rx_script = src.rx_script();
    _rx_script_args->clear();
    _rx_script_args->append(src.rx_script_args());

    _frames_filename = "";
    _baseband_filename = "";

    _postproc_script = src.postproc_script();
    _postproc_script_args->clear();
    _postproc_script_args->append(src.postproc_script_args());

    _flags = src.flags();    

    free_private();

    return *this;
}

//---------------------------------------------------------------------------
void TSatScript::assign_constants(void)
{
    if(!constants) {
       constants = new QStringList;

       // add here constants that CAN NOT be embedded or DON'T HAVE parameters ie {CONSTANT}
       constants->append("{SATFREQ}");
       constants->append("{SATNAME}");

       // add here constants that CAN be be embedded or HAVE parameters {CONSTANT_NAME:param1, param2, etc
       constants->append("{DATETIME:");
       constants->append("{DIR:");
   }

    if(!commands) {
       commands = new QStringList;

       commands->append("out-file");
       commands->append("baseband-dir");
       commands->append("baseband-file-ext");
       commands->append("frames-dir");
       commands->append("frames-file-ext");
   }

    if(!tmpstr)
        tmpstr = (char *) malloc(SS_TMPSTR_SIZE + 1);

}

//---------------------------------------------------------------------------
void TSatScript::zero(void)
{
    _downlink = "0";

    _rx_script = "";
    _rx_script_args->clear();

    _frames_filename = "";
    _baseband_filename = "";

    _postproc_script = "";
    _postproc_script_args->clear();

    _flags = 0;

    free_private();
}

//---------------------------------------------------------------------------
QString TSatScript::downlink(void) const
{
    QString rc;
    double  dl;

    if(_downlink.isEmpty())
        return "0";
    else {
        dl = atof(_downlink.toStdString().c_str());

        return (dl <= 0) ? "0":_downlink;
    }
}

//---------------------------------------------------------------------------
void TSatScript::flag(int flag_, bool on)
{
    _flags &= ~flag_;
    _flags |= on ? flag_:0;
}

//---------------------------------------------------------------------------
bool TSatScript::flag(int flag_)
{
    return (_flags & flag_) ? true:false;
}

//---------------------------------------------------------------------------
// modes&1 = read files
void TSatScript::readSettings(QSettings *reg, int modes)
{
    reg->beginGroup("Script");

    downlink(reg->value("Downlink", "").toString());
    flags(reg->value("Flags", 0).toInt());

    reg->beginGroup("RX");
      rx_script(reg->value("Script", "").toString());
      rx_script_args(readArgSettings(reg, "Arguments"));
    reg->endGroup();

    reg->beginGroup("Post-RX");
      postproc_script(reg->value("Script", "").toString());
      postproc_script_args(readArgSettings(reg, "Arguments"));
    reg->endGroup();

    if(modes & 1) {
        reg->beginGroup("Files");
          _frames_filename = reg->value("Frames", "").toString();
          _baseband_filename = reg->value("Baseband", "").toString();
        reg->endGroup();
    }

    reg->endGroup();
}

//---------------------------------------------------------------------------
void TSatScript::writeSettings(QSettings *reg, int /*modes*/)
{

    reg->beginGroup("Script");

    reg->setValue("Downlink", _downlink);
    reg->setValue("Flags", _flags);

    reg->beginGroup("RX");
      reg->setValue("Script", _rx_script);
      writeArgSettings(reg, "Arguments", _rx_script_args);
    reg->endGroup();

    reg->beginGroup("Post-RX");
      reg->setValue("Script", _postproc_script);
      writeArgSettings(reg, "Arguments", _postproc_script_args);
    reg->endGroup();

    reg->endGroup();
}

//---------------------------------------------------------------------------
void TSatScript::writeArgSettings(QSettings *reg, QString key, QStringList *args)
{
    QString str;
    int i;

    reg->beginGroup(key);

    for(i=0; i<args->count(); i++) {
        str.sprintf("%d", i);
        reg->setValue(str, args->at(i));
    }

    reg->endGroup();
}

//---------------------------------------------------------------------------
QStringList TSatScript::readArgSettings(QSettings *reg, QString key)
{
    QStringList args;
    QString arg_key, arg;
    int i=0;

    reg->beginGroup(key);

    while(true) {
        arg_key.sprintf("%d", i++);
        arg = reg->value(arg_key, "").toString();
        if(arg.isEmpty())
            break;
        else
            args.append(arg);
    }

    reg->endGroup();

    return args;
}

//---------------------------------------------------------------------------
QStringList TSatScript::rx_default_script_args(void) const
{
    QStringList sl;

    sl.append("--side A");
    sl.append("--decim 32");
    sl.append("--gain 25");
    sl.append("--freq {SATFREQ}");
    sl.append("--satellite {SATNAME}");
    sl.append("--sync-check 0");
    sl.append("out-file {DATETIME:yyyy-MM-ddThhmmss}-{SATNAME}");
    sl.append("baseband-dir {DIR:~/hrpt/baseband/{SATNAME}/{DATETIME:yyyy}/{DATETIME:MMMM}/{DATETIME:dd-dddd}}");
    sl.append("baseband-file-ext .dat");
    sl.append("frames-dir {DIR:~/hrpt/frames/{SATNAME}/{DATETIME:yyyy}/{DATETIME:MMMM}/{DATETIME:dd-dddd}}");
    sl.append("frames-file-ext .hrpt");

    return sl;
}

//---------------------------------------------------------------------------
QStringList TSatScript::postproc_default_script_args(void) const
{
    QStringList sl;

    sl.append("--decim 32");
    sl.append("--sync-check 0");
    sl.append("--satellite {SATNAME}");
    sl.append("out-file {DATETIME:yyyy-MM-ddThhmmss}-{SATNAME}");
    sl.append("frames-dir {DIR:~/hrpt/frames/{SATNAME}/{DATETIME:yyyy}/{DATETIME:MMMM}/{DATETIME:dd-dddd}}");
    sl.append("frames-file-ext .hrpt");

    return sl;
}

//---------------------------------------------------------------------------
// mode & 1 = test only, don't create dirs
QString TSatScript::get_rx_command(const QString& satname, const double frequency, bool *error, int mode)
{
    QString rc;

    _satname = satname;
    _satname.replace(" ", "-");
    _frequency = frequency;

    *error = true;

    if(!rx_srcrip_enable())
        return "Error: RX Script is not enabled!";

    if(satname.isEmpty())
        return "Error: No satellite name!";
    if(frequency <= 0)
        return "Error: Undefined frequency!";

    now = QDateTime::currentDateTime();

    _frames_filename   = "";
    _baseband_filename = "";

    rc = getcommand(_rx_script, _rx_script_args, error, mode);

    if(!*error) {
        if(postproc_srcrip_enable() && _baseband_filename.isEmpty()) {
            *error = true;
            return "\nBaseband file must be defined when post RX processing script is enabled!";
        }
    }

    return rc;
}

//---------------------------------------------------------------------------
// mode & 1 = test only, don't create dirs
QString TSatScript::get_postproc_command(bool *error, int mode)
{
    QString rc;

    *error = true;

    if(!postproc_srcrip_enable())
        return "Error: Post RX Script is not enabled!";

    if(!rx_srcrip_enable())
        return "Error: RX Script must also be enabled!";

    if(_baseband_filename.isEmpty())
        return "Error: Baseband file is empty or not defined in the RX script! Run the RX script first!";

    rc = getcommand(_postproc_script, _postproc_script_args, error, mode);

    return rc;
}

//---------------------------------------------------------------------------
// mode & 1 = test only, don't create dirs
QString TSatScript::getcommand(QString script, QStringList *args, bool *error, int mode)
{
    QString rc, arg;
    QString out_file, frames_dir, frames_ext;
    QString baseband_dir, baseband_ext;
    int     i, index, len;

    *error = true;

    if(script.isEmpty())
        return "Error: No script file!";
#if 0
    QFileInfo fi(script);
    if(!fi.exists())
        return "Error: File not found: " + script;
#endif

    if(!args->count())
        return "Error: No arguments: " + script;

    *error = false;
    rc = script;

    assign_constants();

    for(i=0; i<args->count(); i++) {
        arg = args->at(i);

        if(arg.contains("~"))
            arg.replace("~", QDir::homePath());

        arg = parse_argument(arg, error, mode);
        arg = arg.trimmed();

        if(arg.isEmpty())
            continue;

        if(*error) {
            rc += " " + arg;
            rc += "\n\nError!";

            return rc;
        }

        index = getcommand_index(arg);
        if(index < 0) {
            rc += " " + arg;
            continue;
        }

        len = commands->at(index).length() + 1;

        // qDebug("\narg: %s, [%s:%d]\n", arg.toStdString().c_str(), __FILE__, __LINE__);

        switch(index) {
        case SS_CMD_OUT_FILE:
            {
                out_file = arg.remove(0, len);
                out_file = out_file.trimmed();
            }
            break;
        case SS_CMD_BASEBAND_DIR:
            {
                //qDebug("\nbaseband dir arg: %s, [%s:%d]", arg.toStdString().c_str(), __FILE__, __LINE__);

                baseband_dir = arg.remove(0, len);
                baseband_dir = baseband_dir.trimmed();

                //qDebug("baseband dir: %s, [%s:%d]", baseband_dir.toStdString().c_str(), __FILE__, __LINE__);

                // remove the trailing slash
                if(baseband_dir.endsWith('/') || baseband_dir.endsWith('\\'))
                    baseband_dir.remove(baseband_dir.length() - 1, 1);

                //qDebug("baseband dir: %s, [%s:%d]\n", baseband_dir.toStdString().c_str(), __FILE__, __LINE__);
            }
            break;
        case SS_CMD_BASEBAND_EXT:
            {
                baseband_ext = arg.remove(0, len);
                baseband_ext = baseband_ext.trimmed();
            }
            break;
        case SS_CMD_FRAMES_DIR:
            {
                frames_dir = arg.remove(0, len);
                frames_dir = frames_dir.trimmed();

                // remove the trailing slash
                if(frames_dir.endsWith('/') || frames_dir.endsWith('\\'))
                    frames_dir.remove(frames_dir.length() - 1, 1);
            }
            break;

        case SS_CMD_FRAMES_EXT:
            {
                frames_ext = arg.remove(0, len);
                frames_ext = frames_ext.trimmed();
            }
            break;

        default:
            continue;
        }
    }

    free_private();

#if 0
    // TODO: add no-outfile option
    if(out_file.isEmpty()) {
        *error = true;
        rc += "\n\nError: No out-file";

        return rc;
    }
#endif

    if(!frames_dir.isEmpty() && !frames_ext.isEmpty())
        _frames_filename = mkfilename(frames_dir, out_file, frames_ext);

    if(!baseband_dir.isEmpty() && !baseband_ext.isEmpty())
        _baseband_filename = mkfilename(baseband_dir, out_file, baseband_ext);

    if(!_frames_filename.isEmpty())
        rc += " --frames-file " + _frames_filename;

    if(!_baseband_filename.isEmpty())
        rc += " --baseband-file " + _baseband_filename;

#if 0
    qDebug("frames file: %s:%d", _frames_filename.toStdString().c_str(), __LINE__);
    qDebug("baseband file: %s:%d", _baseband_filename.toStdString().c_str(), __LINE__);
#endif

    return rc;
}

//---------------------------------------------------------------------------
// mode&1 = testing
QString TSatScript::parse_argument(QString arg, bool *error, int mode)
{
    QString str, constant;
    int     index, start_pos, end_pos, pos, len;

    if(*error)
        return "";

    index = getconstant_index(arg);
    if(index < 0)
        return arg; // not in our scope

    constant  = constants->at(index);
    start_pos = arg.indexOf(constant);
    end_pos   = arg.indexOf('}', start_pos + 1);

    //qDebug("in arg: %s, line:%d", arg.toStdString().c_str(), __LINE__);

    if(start_pos < 0 || end_pos <= (start_pos + constant.length() - 2)) {
        *error = true;
        arg += "\nBogus argument!";

        return arg; // erroneous argument
    }

    switch(index)
    {
        // check first arguments that can not be embedded, ie {CONSTANT_NAME}
    case SS_INDEX_SATNAME:
    case SS_INDEX_SATFREQ:
        {
            switch(index)
            {
                case SS_INDEX_SATFREQ:
                {
                    // frequency in Hz
                    str.sprintf("%.0f", _frequency * 1.0e6);

                    break;
                }
            case SS_INDEX_SATNAME:
                {
                    str.sprintf("%s", _satname.toStdString().c_str());

                    break;
                }

            default:
                {
                    *error = true;

                    return "Wrong index";
                }
            }

            arg = arg.remove(constant);
            arg = arg.insert(start_pos, str);

            //qDebug("out arg 1: %s, line:%d", arg.toStdString().c_str(), __LINE__);

            return parse_argument(arg, error, mode);
        }

        // check embedded arguments, ie {CONSTANT_NAME:arguments {CONSTANT_NAME} arguments...}
    case SS_INDEX_DATETIME:
    case SS_INDEX_DIR:
        {
            pos = start_pos + constant.length();
            len = end_pos - pos;

            if(len <= 1) {
                *error = true;
                arg += "\nBogus parameter, too short!";

                return arg;
            }

            str = substring(arg, pos, len);

            //qDebug("SS_INDEX_XX: %s, line:%d", str.toStdString().c_str(), __LINE__);

            switch(index)
            {
            case SS_INDEX_DATETIME:
                {
                    str = now.toString(str);
                    //qDebug("arg datetime 1: %s, line:%d", str.toStdString().c_str(), __LINE__);

                    arg = arg.remove(start_pos, end_pos - start_pos + 1);
                    //qDebug("arg datetime 2: %s, line:%d", arg.toStdString().c_str(), __LINE__);

                    arg = arg.insert(start_pos, str);
                    //qDebug("arg datetime 3: %s, line:%d\n", arg.toStdString().c_str(), __LINE__);

                    break;
                }

            case SS_INDEX_DIR:
                {
                    //qDebug("dir: %s, line:%d", str.toStdString().c_str(), __LINE__);

                    // get the directory path
                    if(!(mode & 1)) {
                        if(!checkdirectory(str)) {
                            *error = true;
                            arg += "\nFailed to create directory: " + str;

                            return arg;
                        }
                    }

                    // remove the constant and ending bracket {DIR: ... }
                    arg = arg.remove(end_pos, 1);
                    arg = arg.remove(constant);

                    break;
                }

            default:
                {
                    *error = true;

                    return "Wrong index";
                }
            }

            // qDebug("arg: %s, line:%d", arg.toStdString().c_str(), __LINE__);

            return parse_argument(arg, error, mode);
        }

    default:
        return "";
    }

    return "";
}

//---------------------------------------------------------------------------
bool TSatScript::checkdirectory(QString path)
{
    QDir qdir(path);

    //qDebug("checkdirectory: %s, %s:%d", path.toStdString().c_str(),  __FILE__, __LINE__);

    if(!qdir.exists()) {
        if(!qdir.mkpath(path)) {
            qDebug("Error: Failed to create directory %s, %s:%d",
                   path.toStdString().c_str(),
                   __FILE__, __LINE__);

            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------
int TSatScript::getconstant_index(QString arg)
{
    return get_list_index(arg, constants);
}

//---------------------------------------------------------------------------
int TSatScript::getcommand_index(QString arg)
{
    return get_list_index(arg, commands);
}

//---------------------------------------------------------------------------
int TSatScript::get_list_index(QString arg, QStringList *sl)
{
    for(int i=0; i<sl->count(); i++)
        if(arg.contains(sl->at(i)))
            return i;

    return -1;
}

//---------------------------------------------------------------------------
bool TSatScript::findcommand(int command_index, QStringList *sl)
{
    if(command_index < 0 || command_index >= commands->count())
        return false;

    QString arg = commands->at(command_index);

    for(int i=0; i<sl->count(); i++)
        if(arg.contains(sl->at(i)))
            return true;

    return false;
}

//---------------------------------------------------------------------------
char *TSatScript::substring(QString arg, int start_pos, int length)
//QString TSatScript::substring(QString arg, int start_pos, int length)
{
    int i, j;

    //qDebug("substring: %s, startpos:%d, length:%d", arg.toStdString().c_str(), start_pos, length);

    for(i=start_pos, j=0; i<arg.length() && j<length; i++, j++)
        tmpstr[j] = arg.at(i).toAscii();

    tmpstr[j] = '\0';

    //qDebug("substring: %s\n", tmpstr);

    return tmpstr;
}

//---------------------------------------------------------------------------
QString TSatScript::mkfilename(QString path, QString name, QString ext)
{
    QString filename;

    if(name.isEmpty())
        return "";

    if(!path.isEmpty())
        filename = path + "/" + name + ext;
    else
        filename = name + ext;

    return filename;
}

//---------------------------------------------------------------------------
