/*
 *      textedit.cpp
 *
 *      Copyright 2008 gpg4usb-team <gpg4usb@cpunk.de>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include "QDebug"
#include "QUrl"
#include "textedit.h"

class QFileDialog;
class QMessageBox;

TextEdit::TextEdit()
{
    countPage         = 0;
    tabWidget = new QTabWidget(this);
    tabWidget->setMovable(true);
    tabWidget->setTabsClosable(true);
    tabWidget->setDocumentMode(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tabWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(removeTab(int)));
//    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(curSyntaxHiglight()));
    newFile();
    setAcceptDrops(true);
}


/*void TextEditor::closeEvent(QCloseEvent *event)
{
    int  curIndex = tabWidget->count();
    bool answ     = true;


    while (curIndex >= 1 && answ == true)
    {
        answ = closeFile();

        curIndex--;
    }


    if (answ == true)
    {
        writeSettings();
        event->accept();
    }
    else
    {
        event->ignore();
    }
}
*/
void TextEdit::newFile()
{
    QString header = "new " +
                     QString::number(++countPage);

    tabWidget->addTab(new EditorPage(), header);
    tabWidget->setCurrentIndex(tabWidget->count() - 1);

//    setCursorPosition();
 }


void TextEdit::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"),
                                                    QDir::currentPath());
    setCurrentFile(fileName);

    if (!fileName.isEmpty())
    {
        EditorPage *page = new EditorPage(fileName);
        QFile file(fileName);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);
			QApplication::setOverrideCursor(Qt::WaitCursor);
			page->getTextPage()->setPlainText(in.readAll());

            QTextDocument *document = page->getTextPage()->document();
            document->setModified(false);

            tabWidget->addTab(page, strippedName(fileName));
            tabWidget->setCurrentIndex(tabWidget->count() - 1);
			QApplication::restoreOverrideCursor();
           //       setCursorPosition();
            //enableAction(true);
        }
        else
        {
			    QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));   
        }
    }
}


void TextEdit::save()
{
    QString fileName = curPage()->getFilePath();

    // åñëè òåêóùàÿ ñòðàíèöà íå ñîäåðæèò
    // ïóòè ê ôàéëó òî
    if (fileName.isEmpty())
    {
        // ñîõðàíÿåì åå ïîä íîâûì èìåíåì
        saveAs();
    }
    else
    {
        // èíà÷å ñîõðàíÿåì ôàéë
        // ïîä òåêóùåì èìåíåì
        saveFile(fileName);
    }
}


bool TextEdit::saveFile(const QString &fileName)
{
    if (fileName.isEmpty())
    {
        return false;
    }


    QFile textFile(fileName);

    // åñëè óäàëîñü îòêðûòü ôàéë òî
    if (textFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream outputStream(&textFile);
        EditorPage *page = curPage();

        // çàïèñûâàåì â íåãî âåñü òåêñò,
        // êîòîðûé áûë íà ñòðàíèöå
        outputStream << page->getTextPage()->toPlainText();

        // ïîìå÷àåì äîêóìåíò ÷òî îí íå èçìåíÿëñÿ
        QTextDocument *document = page->getTextPage()->document();
        document->setModified(false);

        int curIndex = tabWidget->currentIndex();
        tabWidget->setTabText(curIndex, strippedName(fileName));

  //      statusBar()->showMessage(tr("File saved"), 2000);

        return true;
    }
    else
    {
    //    statusBar()->showMessage(tr("Error save file"), 2000);

        return false;
    }
}


bool TextEdit::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"),
                                                    QDir::currentPath());

    return saveFile(fileName);
}


bool TextEdit::closeFile()
{
    if (tabWidget->count() != 0)
    {

        if (maybeSave())
        {
            int tabIndex = tabWidget->currentIndex();
            tabWidget->setCurrentIndex(tabIndex);

            curPage()->close();

            tabWidget->removeTab(tabIndex);


            if (tabWidget->count() == 0)
            {
 //               enableAction(false);
            }

            return true;
        }

        return false;
    }
    return false;
}


void TextEdit::removeTab(int index)
{
    if (tabWidget->count() != 0)
    {
        if (maybeSave())
        {
            tabWidget->setCurrentIndex(index);

            curPage()->close();

            tabWidget->removeTab(index);
        }
    }


    if (tabWidget->count() == 0)
    {
      //  enableAction(false);
    }
}

void TextEdit::cut()
{
    curTextPage()->cut();
}


void TextEdit::copy()
{
    curTextPage()->copy();
}


void TextEdit::paste()
{
    curTextPage()->paste();
}


void TextEdit::undo()
{
    curTextPage()->undo();
}


void TextEdit::redo()
{
    curTextPage()->redo();
}

void TextEdit::selectAll()
{
    curTextPage()->selectAll();
}

bool TextEdit::maybeSave()
{
    EditorPage *page = curPage();

    // åñëè íå îñòàëîñü íè îäíîé çàêëàäêè òî
    if (page == 0)
    {
        return false;
    }

    QTextDocument *document = page->getTextPage()->document();

    // åñëè áûëè èçìåíåíèÿ â òåêñòå òî
    if (document->isModified())
    {
        int result = QMessageBox::information(this, tr("Save"),tr("Save file ?"),
                                              QMessageBox::Yes, QMessageBox::No,
                                              QMessageBox::Cancel);


        if (result == QMessageBox::Yes)
        {
            // ïîëó÷àåì ïóòü äî ôàéëà êîòîðûé õðàíèò êàæäàÿ ñòðàíèöà
            QString filePath = page->getFilePath();

            // åñëè ýòî íîâûé ôàéë è ïóòè ó íåãî íåò, òî
            if (filePath == "")
            {
                // äàåì ïîëüçîâàòåëþ ñîõðàíèòü åãî
                // ïîä íîâûì èìåíåì
                return saveAs();
            }
            else
            {
                // èíà÷å ñîõðàíÿåì ñ òåêóùèì èìåíåì
                return saveFile(filePath);
            }
        }
        else if (result == QMessageBox::No)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return true;
}


QPlainTextEdit* TextEdit::curTextPage()
{
    EditorPage *curTextPage = qobject_cast<EditorPage *>(tabWidget->currentWidget());

    return curTextPage->getTextPage();
}


EditorPage* TextEdit::curPage()
{
    EditorPage *curPage = qobject_cast<EditorPage *>(tabWidget->currentWidget());

    return curPage;
}

void TextEdit::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain"))
        qDebug() << "enter textedit drag action";

        event->acceptProposedAction();
}

void TextEdit::dropEvent(QDropEvent* event)
{
    curTextPage()->setPlainText(event->mimeData()->text());

    qDebug() << "enter textedit drop action";
    qDebug() << event->mimeData()->text();
    foreach (QUrl tmp, event->mimeData()->urls())
    {
        qDebug() << "hallo" << tmp;
    }

    //event->acceptProposedAction();
}

void TextEdit::quote()
{
    QTextCursor cursor(curTextPage()->document());

    // beginEditBlock and endEditBlock() let operation look like single undo/redo operation
    cursor.beginEditBlock();
    cursor.setPosition(0);
    cursor.insertText("> ");
    while (!cursor.isNull() && !cursor.atEnd()) {
        cursor.movePosition(QTextCursor::EndOfLine);
        cursor.movePosition(QTextCursor::NextCharacter);
        if(!cursor.atEnd())
            cursor.insertText("> ");
    }
    cursor.endEditBlock();

}

bool TextEdit::isKey(QString key)
{
    qDebug() << key.contains("-----BEGIN PGP PUBLIC KEY BLOCK-----", Qt::CaseSensitive);
    return true;
}


void TextEdit::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }
    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    curTextPage()->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
   // statusBar()->showMessage(tr("File loaded"), 2000);
}

void TextEdit::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    curTextPage()->document()->setModified(false);
    setWindowModified(false);

    QString shownName;
    if (curFile.isEmpty())
        shownName = "untitled.txt";
    else
        shownName = strippedName(curFile);

    //setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(qApp->applicationName()));
}

QString TextEdit::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void TextEdit::print()
{
#ifndef QT_NO_PRINTER
    QTextDocument *document = curTextPage()->document();
    QPrinter printer;

    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (dlg->exec() != QDialog::Accepted)
        return;

    document->print(&printer);

    //statusBar()->showMessage(tr("Ready"), 2000);
#endif
}
