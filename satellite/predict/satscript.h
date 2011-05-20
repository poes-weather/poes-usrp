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

#ifndef SATSCRIPT_H
#define SATSCRIPT_H

//---------------------------------------------------------------------------
#include <QString>
#include <QStringList>
#include <QDateTime>

//---------------------------------------------------------------------------
#define SS_ENABLE_REC_SCRIPT            1
#define SS_ENABLE_POSTPROC_SCRIPT       2
#define SS_ENABLE_DC                    4
#define SS_SAT_ACTIVE                   8

//---------------------------------------------------------------------------
class QSettings;


//---------------------------------------------------------------------------
class TSatScript
{
public:
    TSatScript(void);
    TSatScript(const TSatScript& src);
    ~TSatScript(void);

    TSatScript& operator = (const TSatScript& src);
    void zero(void);

    bool active(void)        { return flag(SS_SAT_ACTIVE); }
    void active(bool active) { return flag(SS_SAT_ACTIVE, active); }

    bool downconvert(void)        { return flag(SS_ENABLE_DC); }
    void downconvert(bool enable) { return flag(SS_ENABLE_DC, enable); }

    QString downlink(void) const;
    void    downlink(const QString& dl) { _downlink = dl; }

    bool         rx_srcrip_enable(void)                  { return flag(SS_ENABLE_REC_SCRIPT); }
    void         rx_srcrip_enable(bool enable)           { flag(SS_ENABLE_REC_SCRIPT, enable); }
    QString      rx_script(void) const                   { return _rx_script; }
    void         rx_script(const QString& script)        { _rx_script = script; }
    QStringList& rx_script_args(void) const              { return *_rx_script_args; }
    void         rx_script_args(const QStringList& args) { *_rx_script_args = args; }
    QStringList  rx_default_script_args(void) const;
    QString      get_rx_command(const QString& satname, const double frequency, bool *error, int mode=0);

    bool         postproc_srcrip_enable(void)                  { return flag(SS_ENABLE_POSTPROC_SCRIPT); }
    void         postproc_srcrip_enable(bool enable)           { flag(SS_ENABLE_POSTPROC_SCRIPT, enable); }
    QString      postproc_script(void) const                   { return _postproc_script; }
    void         postproc_script(const QString& script)        { _postproc_script = script; }
    void         postproc_script_args(const QStringList& args) { *_postproc_script_args = args; }
    QStringList& postproc_script_args(void) const              { return *_postproc_script_args; }
    QStringList  postproc_default_script_args(void) const;
    QString      get_postproc_command(bool *error, int mode=0);

    QString      frames_filename(void) const { return _frames_filename; }
    QString      baseband_filename(void) const { return _baseband_filename; }

    int  flags(void) const { return _flags; }
    void flags(int flags_) { _flags = flags_; }

    void readSettings(QSettings *reg, int modes=0);
    void writeSettings(QSettings *reg, int modes=0);

protected:
    void flag(int flag_, bool on);
    bool flag(int flag_);

    void        writeArgSettings(QSettings *reg, QString key, QStringList *args);
    QStringList readArgSettings(QSettings *reg, QString key);

    void assign_constants(void);
    void free_private(void);
    int  get_list_index(QString arg, QStringList *sl);

    QString getcommand(QString script, QStringList *args, bool *error, int mode);

    QString parse_argument(QString arg, bool *error, int mode);
    int     getconstant_index(QString arg);
    int     getconstant_startpos(QString arg, QString &constant);
    char    *substring(QString arg, int start_pos, int length);
    QString mkfilename(QString path, QString name, QString ext);

    int     getcommand_index(QString arg);
    bool    findcommand(int command_index, QStringList *sl);
    bool    checkdirectory(QString path);


private:
    QString     _downlink;
    QString     _rx_script, _postproc_script;
    QStringList *_rx_script_args, *_postproc_script_args;
    int         _flags;

    QString     _frames_filename, _baseband_filename;
    QStringList *constants, *commands;
    QString     _satname;
    double      _frequency;

    QDateTime  now;
    char       *tmpstr;

};

#endif // TSATSCRIPT_H
