#include "test.hh"
#include "main.cc"
#include <QList>
#include <QTextStream>

void compare(ChatDialog *a, ChatDialog *b) {
	QMap<QString, QVector<QString> > msgTable_a = (*a).msgTable;
	QMap<QString, QVector<QString> > msgTable_b = (*b).msgTable;

	// for iterator in a's msgTable, verify b's corresponding results.
	QMap<QString, QVector<QString> >::const_iterator i = msgTable_a.constBegin();
	while (i != msgTable_a.constEnd()) {
		QString key = i.key();
		QVector<QString> v = i.value();
		// compare whether the msg key in msgTable
		QVERIFY2(msgTable_b.contains(key),"App-B's msgTable does not contain this key from App-A.");
		// compare whether the msg breaks or not
		QVERIFY2(v.size() == msgTable_b[key].size(),"msg length of App-A is not consistent with that in App-B.");
		//compare the message letter by letter
		for (int j = 0; j < v.size(); ++j) {
			QVERIFY2((v[j] == msgTable_b[key][j]), "msg content of App-A is not consistent with that in App-B.");
		}
    	++i;
	}

	// iterator in b's msgTable, if b's keys is identical with a's, then all are the same
	i = msgTable_b.constBegin();
	while (i != msgTable_b.constEnd()) {
		QVERIFY2(msgTable_a.contains(i.key()), "App-A's msgTable does not contain this key from App-B.");
		//if (!msgTable_a.contains(i.key())) return false;
    	++i;
	}
}



QString myReadFile(QString filename) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
     return NULL;
  }
 
  QByteArray total;
  QByteArray line;
  while (!file.atEnd()) {
     line = file.read(1024);
     total.append(line);
  }
 
  return QString(total);
}




void TestGui::testGui()
{
    ChatDialog dialog1;
    dialog1.show();

    ChatDialog dialog2;
    dialog2.show();

   	ChatDialog dialog3;
    dialog3.show();

    ChatDialog dialog4;
    dialog4.show();

    QList<ChatDialog *> dialogs;
    dialogs << &dialog1 << &dialog2 << &dialog3 << &dialog4;


    QList<QString> testStrings;
    QString qs_1 = "hello";
    QString qs_3 = "ñóǹ äŝçíì 汉语/漢語  华语/華語 Huáyǔ; 中文 Zhōngwén 漢字仮名交じり文 Lech Wałęsa æøå";
    QString qs_2 = " ";
    testStrings << qs_1 << qs_2 << qs_3;

    QString fname = "hell.txt";
	QString aFile = myReadFile(fname);
	qDebug() << aFile;

    for (int a = 0; a < testStrings.size(); a++){
    	qDebug() << testStrings[a];
    }

    for (int i = 0; i < dialogs.size(); ++i) {
    	// for (int j = 0; j < testStrings.size(); ++j){
    	// 	(*dialogs[i]).textline->insert(testStrings[j]);
    	// 	QTest::keyClick((*dialogs[i]).textline, Qt::Key_Enter);
    	// }
    	(*dialogs[i]).textline->insert(aFile);
    	QTest::keyClick((*dialogs[i]).textline, Qt::Key_Enter);

	}

	//qDebug() << "Wait begin";
	QWARN("Wait begin\n");

	QTest::qWait(20000);

	//qDebug() << "Test begin";
	QWARN("Test begin\n");

	for (int i = 0; i < dialogs.size(); ++i) {
		for (int j = i + 1; j < dialogs.size(); ++j) {
			compare(dialogs[i], dialogs[j]);
		}
	}

	//qDebug() << "Successful\n";
	QWARN("Successful\n");
}




QTEST_MAIN(TestGui)
/*
int main(int argc, char **argv)
{
    // Initialize Qt toolkit
    QApplication app(argc,argv);

    // Create an initial chat dialog window
   	ChatDialog dialog1;
    dialog1.show();

    ChatDialog dialog2;
    dialog2.show();

   	ChatDialog dialog3;
    dialog3.show();

    ChatDialog dialog4;
    dialog4.show();

    QList<ChatDialog *> dialogs;
    dialogs << &dialog1 << &dialog2 << &dialog3 << &dialog4;

    for (int i = 0; i < dialogs.size(); ++i) {
    	QTest::keyClicks((*dialogs[i]).textline, QString("hello I am ") + QString::number(i));
    	QTest::keyClick((*dialogs[i]).textline, Qt::Key_Enter);
	}

	qDebug() << "Wait begin";

	QTest::qWait(20000);

	qDebug() << "Test begin";

	for (int i = 0; i < dialogs.size(); ++i) {
		for (int j = i + 1; j < dialogs.size(); ++j) {
			if (!compare(dialogs[i], dialogs[i])) {
				qDebug() << "Fail\n";
			}
		}
	}

	qDebug() << "Successful\n";


    // Enter the Qt main loop; everything else is event driven
    return app.exec();
}
*/