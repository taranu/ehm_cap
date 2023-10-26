//============================================================================
// Name        : filereader.cpp
// Author      : Dan Taranu
// Version     : 0.1
// Copyright   : (C) Dan Taranu, 2009-2021
// Description : EHM File Converter and Cap Calculator
//============================================================================

#include <algorithm>
#include <assert.h>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
using namespace std;

typedef long long int caphit;
typedef unsigned int uint;

const caphit MINCAPHIT = 320000;
const caphit MINCAPHITCURR = 600000;
const size_t NTEAMS = 30;
const size_t NGAMES = 82;
const caphit MAXCAP = 60e6;
const caphit MAXAHLSALARY = 8e5;
const double WAIVERAGE = 23;
const size_t MINNPRO = 22;
const int YEAR_FIRST = 2023;

const string EHMSEP = "  ";

const string TEAMNAMES[NTEAMS] = {"ANA","CBJ","BOS","BUF","CGY","CAR","CHI","COL","WPG","DAL","DET","EDM","FLA","LA","MIN","MTL","NYI","NYR","NAS","NJ","OTT","PHI","ARZ","PIT","SJ","STL","TB","TOR","VAN","WAS"};
const string POSITIONS[5] = {"G","D","LW","C","RW"};
const string HANDS[2] = {"R","L"};
const size_t NPERFORMANCE = 4;
const size_t NRECORDS = 3;
const size_t NOPTIONS = 3;
const size_t NSTATUSES = 6;
const size_t NMISC = 5;
const size_t NSTREAKS = 5;

string SPACEREPLACE = ".";

class Player
{
	/*
	 * Attributes go: sh pl st ch po hi sk en pe fa le sr fi (13 total)
	 */
	private:
		static const int STATS = 13;
		static const char SEP = ',';
		int ratings[STATS];
		int ceilings[STATS];

		int pot;
		int con;
		int gre;
		int click;
		int team;
		int position;
		int country;
		int hand;
		int byear;
		int bday;
		int bmonth;
		caphit salary;
		int years;
		int draftyear;
		int draftedby;
		int draftround;
		int rights;
		int id;

		string dashcode;
		string firstName;
		string lastName;
		string performance;

		int thisweek[NPERFORMANCE];
		int thismonth[NPERFORMANCE];
		int records[NRECORDS];
		int options[NOPTIONS];
		int status[NSTATUSES];
		string scout1;
		string scout2;
		string scout3;
		int attitude;
		int altpos;
		int nhlrights;
		int injuryprone;
		int draftedoverall;
		int misc[NMISC];
		int height;
		int weight;
		int orgstatus;
		int streaks[NSTREAKS];
		string draftedstatus;
		string EHMversion;

	public:
		Player(ifstream& inputFile, bool fromPlayersEHM, char * tempdata, const int tempdataSize, int iId)
		{
			id = iId;
			if(fromPlayersEHM)
			{
				for(int i = 1; i < STATS; i++)
				{
					inputFile >> ratings[i];
				}
				inputFile >> pot;
				inputFile >> con;
				inputFile >> gre;
				inputFile >> ratings[0];
				inputFile >> click;
				inputFile >> team;
				inputFile >> position;
				inputFile >> country;
				inputFile >> hand;
				inputFile >> byear;
				inputFile >> bday;
				inputFile >> bmonth;
				inputFile >> salary;
				inputFile >> years;
				inputFile >> draftyear;
				inputFile >> draftround;
				inputFile >> draftedby;
				inputFile >> rights;

				for(size_t i = 0; i < NPERFORMANCE; i++) inputFile >> thisweek[i];
				for(size_t i = 0; i < NPERFORMANCE; i++) inputFile >> thismonth[i];
				for(size_t i = 0; i < NRECORDS; i++) inputFile >> records[i];
				for(size_t i = 0; i < NOPTIONS; i++) inputFile >> options[i];
				for(size_t i = 0; i < NSTATUSES; i++) inputFile >> status[i];

				// Reads in the extra space after the last status
				inputFile.getline(tempdata,tempdataSize);
				inputFile.getline(tempdata,tempdataSize);
				scout1 = string(tempdata);
				inputFile.getline(tempdata,tempdataSize);
				scout2 = string(tempdata);
				inputFile.getline(tempdata,tempdataSize);
				scout3 = string(tempdata);

				for(size_t i = 0; i < NMISC; i++) inputFile >> misc[i];
				inputFile >> weight;
				inputFile >> height;
				inputFile >> orgstatus;
				for(size_t i = 0; i < NSTREAKS; i++) inputFile >> streaks[i];
				// another extra space
				inputFile.getline(tempdata,tempdataSize);
				inputFile.getline(tempdata,tempdataSize);
				dashcode = string(tempdata);

				inputFile >> firstName;
				// explicitly read the space between first and last name
				inputFile.get();
				inputFile.getline(tempdata,tempdataSize);
				lastName = string(tempdata);

				inputFile.getline(tempdata,tempdataSize);
				performance = string(tempdata);
				inputFile.getline(tempdata,tempdataSize);
				draftedstatus = string(tempdata);
				inputFile.getline(tempdata,tempdataSize);

				char ceil[3];
				for(size_t i = 0; i < STATS; i++)
				{
					for(int j=0; j<3; j++)
					{
						ceil[j] = tempdata[i*3+j];
					}
					ceilings[i] = atoi(ceil);
				}

				inputFile.getline(tempdata,tempdataSize);
				EHMversion = string(tempdata);
				inputFile.getline(tempdata,tempdataSize);

				inputFile >> attitude;
				inputFile >> altpos;
				inputFile >> nhlrights;
				inputFile >> injuryprone;
				inputFile >> draftedoverall;
			}
			else
			{
				std::string buf;
				try
				{
					for(int i = 1; i < STATS; i++)
					{
						getline(inputFile, buf, SEP);
						ratings[i] = atol(buf.c_str());
					}
					getline(inputFile, buf, SEP); pot = atol(buf.c_str());
					getline(inputFile, buf, SEP); con = atol(buf.c_str());
					getline(inputFile, buf, SEP); gre = atol(buf.c_str());
					getline(inputFile, buf, SEP); ratings[0] = atol(buf.c_str());
					getline(inputFile, buf, SEP); click = atol(buf.c_str());
					getline(inputFile, buf, SEP); team = atol(buf.c_str());
					getline(inputFile, buf, SEP); position = atol(buf.c_str());
					getline(inputFile, buf, SEP); country = atol(buf.c_str());
					getline(inputFile, buf, SEP); hand = atol(buf.c_str());
					getline(inputFile, buf, SEP); byear = atol(buf.c_str());
					getline(inputFile, buf, SEP); bday = atol(buf.c_str());
					getline(inputFile, buf, SEP); bmonth = atol(buf.c_str());
					getline(inputFile, buf, SEP); salary = atol(buf.c_str());
					getline(inputFile, buf, SEP); years = atol(buf.c_str());
					getline(inputFile, buf, SEP); draftyear = atol(buf.c_str());
					getline(inputFile, buf, SEP); draftround = atol(buf.c_str());
					getline(inputFile, buf, SEP); draftedby = atol(buf.c_str());
					getline(inputFile, buf, SEP); rights = atol(buf.c_str());
					for(size_t i = 0; i < NPERFORMANCE; i++)
					{
						getline(inputFile, buf, SEP);
						thisweek[i] = atol(buf.c_str());
					}
					for(size_t i = 0; i < NPERFORMANCE; i++)
					{
						getline(inputFile, buf, SEP);
						thismonth[i] = atol(buf.c_str());
					}
					for(size_t i = 0; i < NRECORDS; i++)
					{
						getline(inputFile, buf, SEP);
						records[i] = atol(buf.c_str());
					}
					for(size_t i = 0; i < NOPTIONS; i++)
					{
						getline(inputFile, buf, SEP);
						options[i] = atol(buf.c_str());
					}
					for(size_t i = 0; i < NSTATUSES; i++)
					{
						getline(inputFile, buf, SEP);
						status[i] = atol(buf.c_str());
					}
					getline(inputFile, scout1, SEP);
					getline(inputFile, scout2, SEP);
					getline(inputFile, scout3, SEP);
					for(size_t i = 0; i < NMISC; i++)
					{
						getline(inputFile, buf, SEP);
						misc[i] = atol(buf.c_str());
					}
					getline(inputFile, buf, SEP); weight = atol(buf.c_str());
					getline(inputFile, buf, SEP); height = atol(buf.c_str());
					getline(inputFile, buf, SEP); orgstatus = atol(buf.c_str());
					for(size_t i = 0; i < NSTREAKS; i++)
					{
						getline(inputFile, buf, SEP);
						streaks[i] = atol(buf.c_str());
					}
					getline(inputFile, dashcode, SEP);
					getline(inputFile, firstName, SEP);
					getline(inputFile, lastName, SEP);
					getline(inputFile, performance, SEP);
					getline(inputFile, draftedstatus, SEP);
					for(size_t i = 0; i < STATS; i++)
					{
						getline(inputFile, buf, SEP);
						ceilings[i] = atol(buf.c_str());
					}
					getline(inputFile, EHMversion, SEP);
					getline(inputFile, buf, SEP); attitude = atol(buf.c_str());
					getline(inputFile, buf, SEP); altpos = atol(buf.c_str());
					getline(inputFile, buf, SEP); nhlrights = atol(buf.c_str());
					getline(inputFile, buf, SEP); injuryprone = atol(buf.c_str());
					getline(inputFile, buf); draftedoverall = atol(buf.c_str());
				}
				catch(exception & e)
				{
					cout << "Standard exception: " << e.what() << endl;
				}
			}
		};

		int getContractLength() const
		{
			return this->years;
		}

		caphit getSalary() const
		{
			return max(this->salary,MINCAPHIT);
		}

		int getTeam() const
		{
			return this->team;
		}

		string getFirstName() const
		{
			return this->firstName;
		}

		string getLastName() const
		{
			return this->lastName;
		}

		int getRating(int rating) const
		{
			assert(rating < STATS);
			return ratings[rating];
		}

		int getRights() const
		{
			return rights;
		}

		int getConsistency() const
		{
			return con;
		}

		int getId() const
		{
			return id;
		}

		// return age as of a certain date. No it's not exact, no I don't care.
		double getAge(int year, int month, int day) const
		{
			double age = year-byear;
			age += (month-bmonth)/12.;
			age += (day-bday)/365.;
			return age;
		}

		void outputDataEHM(ofstream& outputFile) const
		{
			outputFile << " ";
			for(int i = 1; i <= 10; i++)
			{
				outputFile << ratings[i];
				if(i < 10) outputFile << EHMSEP;
			}
			outputFile << " " << endl;
			outputFile << " " << ratings[11] << EHMSEP << ratings[12] << EHMSEP;
			outputFile << pot << EHMSEP;
			outputFile << con << EHMSEP;
			outputFile << gre << EHMSEP;
			outputFile << ratings[0] << EHMSEP;
			outputFile << click << EHMSEP;
			outputFile << team << EHMSEP;
			outputFile << position << EHMSEP;
			outputFile << country << EHMSEP;
			outputFile << hand << " " << endl;
			outputFile << " " << byear << EHMSEP;
			outputFile << bday << EHMSEP;
			outputFile << bmonth << EHMSEP;
			outputFile << salary << EHMSEP;
			outputFile << years << EHMSEP;
			outputFile << draftyear << EHMSEP;
			outputFile << draftround << EHMSEP;
			outputFile << draftedby << EHMSEP;
			outputFile << rights << " " << endl << " ";

			for(size_t i = 0; i < NPERFORMANCE; i++)
			{
				outputFile << thisweek[i];
				if(i == (NPERFORMANCE-1)) outputFile << " " << endl << " ";
				else outputFile << EHMSEP;
			}
			for(size_t i = 0; i < NPERFORMANCE; i++)
			{
				outputFile << thismonth[i];
				if(i == (NPERFORMANCE-1)) outputFile << " " << endl << " ";
				else outputFile << EHMSEP;
			}
			for(size_t i = 0; i < NRECORDS; i++)
			{
				outputFile << records[i] << EHMSEP;
			}
			for(size_t i = 0; i < NOPTIONS; i++)
			{
				outputFile << options[i];
				if(i == (NOPTIONS-1)) outputFile << " " << endl << " ";
				else outputFile << EHMSEP;
			}
			for(size_t i = 0; i < NSTATUSES; i++)
			{
				outputFile << status[i];
				if(i == (NSTATUSES-1)) outputFile << " " << endl;
				else outputFile << EHMSEP;
			}

			outputFile << scout1 << endl;
			outputFile << scout2 << endl;
			outputFile << scout3 << endl;

			for(size_t i = 0; i < NMISC; i++)
			{
				if(misc[i] >= 0) outputFile << " ";
				outputFile << misc[i] << " ";
				if(i == 4) outputFile << " ";
			}
			outputFile << weight << EHMSEP;
			outputFile << height << EHMSEP;
			outputFile << orgstatus << " " << endl << " ";
			for(size_t i = 0; i < NSTREAKS; i++)
			{
				outputFile << streaks[i];
				if(i == (NSTREAKS-1)) outputFile << " " << endl;
				else outputFile << EHMSEP;
			}

			outputFile << dashcode << endl;
			outputFile << firstName << " " << lastName << endl;
			outputFile << performance << endl << draftedstatus << endl;

			char ceil[3];
			for(size_t i = 0; i < STATS; i++)
			{
				sprintf(ceil, "%03d", ceilings[i]);
				outputFile << ceil;
			}

			outputFile << endl << EHMversion << endl << EHMversion << endl;
			if(attitude >= 0) outputFile << " ";
			outputFile << attitude << EHMSEP << altpos << EHMSEP << nhlrights;
			outputFile << EHMSEP << injuryprone << EHMSEP << draftedoverall << " " << endl;
		}

		void outputDataCSV(ofstream& outputFile, size_t row) const
		{
			for(size_t i = 1; i < STATS; i++) outputFile << ratings[i] << SEP;
			outputFile << pot << SEP;
			outputFile << con << SEP;
			outputFile << gre << SEP;
			outputFile << ratings[0] << SEP;
			outputFile << click << SEP;
			outputFile << team << SEP;
			outputFile << position << SEP;
			outputFile << country << SEP;
			outputFile << hand << SEP;
			outputFile << byear << SEP;
			outputFile << bday << SEP;
			outputFile << bmonth << SEP;
			outputFile << salary << SEP;
			outputFile << years << SEP;
			outputFile << draftyear << SEP;
			outputFile << draftround << SEP;
			outputFile << draftedby << SEP;
			outputFile << rights << SEP;

			for(size_t i = 0; i < NPERFORMANCE; i++) outputFile << thisweek[i] << SEP;
			for(size_t i = 0; i < NPERFORMANCE; i++) outputFile << thismonth[i] << SEP;
			for(size_t i = 0; i < NRECORDS; i++) outputFile << records[i] << SEP;
			for(size_t i = 0; i < NOPTIONS; i++) outputFile << options[i] << SEP;
			for(size_t i = 0; i < NSTATUSES; i++) outputFile << status[i] << SEP;
			outputFile << scout1 << SEP << scout2 << SEP << scout3 << SEP;

			for(size_t i = 0; i < NMISC; i++) outputFile << misc[i] << SEP;
			outputFile << weight << SEP << height << SEP << orgstatus << SEP;
			for(size_t i = 0; i < NSTREAKS; i++) outputFile << streaks[i] << SEP;
			outputFile << dashcode << SEP;
			outputFile << firstName << SEP << lastName << SEP;
			outputFile << performance << SEP << draftedstatus << SEP;

			for(int i = 0; i < STATS; i++) outputFile << ceilings[i] << SEP;

			outputFile << EHMversion << SEP << attitude << SEP;
			outputFile << altpos << SEP << nhlrights << SEP;
			outputFile << injuryprone << SEP << draftedoverall << SEP << row << endl;
		};
};

bool higherSalary(Player* i, Player* j)
{
	return i->getSalary() > j->getSalary();
}

class SalaryBrackets
{
	private:
		bool initialized;
		bool usingBrackets;
		int novbrackets;
		int * ovbrackets;
		int * nsalarybrackets;
		double ** salarybrackets;
		double ** raises;
		void deallocate()
		{
			if(usingBrackets)
			{
				delete[] raises[novbrackets-1];
				for(int ovbracket = novbrackets-2; ovbracket >= 0; --ovbracket)
				{
					delete[] salarybrackets[ovbracket];
					delete[] raises[ovbracket];
				}

				delete[] raises;
				delete[] salarybrackets;

				delete[] ovbrackets;
				delete[] nsalarybrackets;
			}

			initialized = false;
		}
		int getOvbracket(double overall)
		{
			for(int ovbracket = 0; ovbracket < novbrackets-1; ovbracket++)
			{
				if(overall < ovbrackets[ovbracket]) return ovbracket;
			}
			return novbrackets-1;
		}
		int getSalaryBracket(int ovbracket, double salary)
		{
			int novsalarybrackets = nsalarybrackets[ovbracket];

			for(int salarybracket = 0; salarybracket < novsalarybrackets-1; salarybracket++)
			{
				if(salary < salarybrackets[ovbracket][salarybracket]) return salarybracket;
			}
			return novsalarybrackets-1;
		}

	public:
		void initialize(istream & inputStream)
		{
			if(!initialized)
			{
				inputStream >> novbrackets;

				ovbrackets = new int[novbrackets-1];
				nsalarybrackets = new int[novbrackets];
				salarybrackets = new double*[novbrackets];
				raises = new double*[novbrackets];

				for(int ovbracket = 0; ovbracket < novbrackets-1; ovbracket++)
				{
					inputStream >> ovbrackets[ovbracket];
				}
				for(int ovbracket = 0; ovbracket < novbrackets; ovbracket++)
				{
					inputStream >> nsalarybrackets[ovbracket];
					salarybrackets[ovbracket] = new double [nsalarybrackets[ovbracket]-1];
					raises[ovbracket] = new double [nsalarybrackets[ovbracket]];
					for(int salarybracket = 0; salarybracket < nsalarybrackets[ovbracket]-1; salarybracket++)
					{
						inputStream >> salarybrackets[ovbracket][salarybracket];
						salarybrackets[ovbracket][salarybracket]*=1.e6;
					}
					for(int salarybracket = 0; salarybracket < nsalarybrackets[ovbracket]; salarybracket++)
					{
						inputStream >> raises[ovbracket][salarybracket];
					}
				}
				initialized = true;
				usingBrackets = true;
			}
		}

		double getSalary(double initialSalary, double overall)
		{
			int ovbracket = getOvbracket(overall);
			int salarybracket = getSalaryBracket(ovbracket, initialSalary);

			double raise = raises[ovbracket][salarybracket];

			return initialSalary * (1.0 + raise/100.);
		}

		SalaryBrackets(istream & inputStream)
		{
			initialized = false;
			initialize(inputStream);
		}

		SalaryBrackets(bool iUsingBrackets = false):usingBrackets(iUsingBrackets)
		{
			if(!usingBrackets) initialized = true;
		}

		~SalaryBrackets()
		{
			deallocate();
		}
};

class PlayerRater
{
	public:
		double maxweight;
		double minweight;

		PlayerRater(double iMaxweight=0.5, double iMinweight=0.5) : maxweight(iMaxweight),minweight(iMinweight)
		{
			;
		}

		double getOFF(const Player & player)
		{
			double off = (player.getRating(1) + player.getRating(2) + player.getRating(3))/3.0;
			return off;
		}

		double getDEF(const Player & player)
		{
			double off = (player.getRating(4) + player.getRating(5) + player.getRating(6))/3.0;
			return off;
		}

		double getOverall(const Player & player)
		{
			double off = getOFF(player);
			double def = getDEF(player);

			return (max(off,def)*maxweight + min(off,def)*minweight);
		}
};

double statBonuses(const Player & player)
{
	double bonus = 0;

	for(int rating=7; rating < 13; rating++)
	{
		if(player.getRating(rating) > 80)
		{
			bonus += 250000;
		}
	}
	if(player.getConsistency() > 80) bonus +=250000;

	return bonus;
}

double getSalary(double currentSalary,double overall)
{
	double minov = 67;
	double midov = 75;
	double maxov = 87;

	double minsalary = 400000;
	double maxsalary = 8e6;

	double qualratio = 1.1;

	if(overall < minov) return currentSalary*qualratio;
	if(overall > maxov) overall = maxov;
	double exponent = 1.0;
	if(overall >= midov)
	{
		exponent -= (overall-midov)/(maxov-midov);
	}
	else
	{
		exponent += (midov-overall)/(midov-minov);
	}

	double basesalary = minsalary + pow((overall-minov)/(maxov-minov),exponent)*(maxsalary-minsalary);

	if(currentSalary >= basesalary) return currentSalary*qualratio;

	double minlevel = 0.7;

	double newsalary = minlevel*basesalary + (qualratio-minlevel)*basesalary*((currentSalary-minsalary)/(basesalary-minsalary));

	return max(newsalary,max(minsalary,currentSalary)*qualratio);
}

void outputNewSalaryInfo(double currentSalary, double overall, ofstream & outputStream, SalaryBrackets & brackets, double statbonus=-1, double otherbonus=-1)
{
	double salary = brackets.getSalary(currentSalary,overall);
	if(statbonus >= 0)
	{
		salary += statbonus;
	}
	if(otherbonus >= 0)
	{
		salary += otherbonus;
	}
	outputStream << salary/1.e6;
	if(statbonus >= 0)
	{
		outputStream << "\t" << statbonus/1.e6;
	}
	if(otherbonus >= 0)
	{
		outputStream << "\t" << otherbonus/1.e6;
	}
}

void outputNewSalaryInfo(double currentSalary, double overall, ofstream & outputStream, double bonus=0)
{
	outputStream << (getSalary(currentSalary,overall)+bonus)/1e6;
}

void outputNewSalaryInfo(const Player & player, double overall, ofstream & outputStream, SalaryBrackets & brackets, double otherBonus = 0)
{
	double currentSalary = player.getSalary();
	outputStream << player.getLastName() << ", " << player.getFirstName() << "\t" << overall << "\t" << currentSalary/1.e6 << "\t";
	outputNewSalaryInfo(currentSalary,overall,outputStream,brackets,statBonuses(player),otherBonus);
	outputStream << endl;
}

void outputNewSalaryInfo(const Player & player, double overall, ofstream & outputStream)
{
	double currentSalary = player.getSalary();
	outputStream << player.getLastName() << ", " << player.getFirstName() << "\t" << overall << "\t" << currentSalary/1.e6 << "\t";
	outputNewSalaryInfo(currentSalary,overall,outputStream);
	outputStream << endl;
}

void outputNewSalariesInfo(const vector<Player> & players, size_t nplayers, string outputFilename, string overallFilename, string bracketFilename="")
{
	ofstream outputFile(outputFilename.c_str());

	outputFile.precision(2);
	outputFile.setf(ios::fixed);

	double minWeight = 0.5;
	double maxWeight = 0.5;

	if(!overallFilename.empty())
	{
		ifstream overallFile(overallFilename.c_str());
		overallFile >> maxWeight;
		overallFile >> minWeight;
		overallFile.close();
	}

	PlayerRater rater(maxWeight,minWeight);

	outputFile << "OV\\SAL\t";
	for(double currentSalary = 400000; currentSalary<9000000; currentSalary+=400000) outputFile << currentSalary/1.e6 << "\t";
	outputFile << endl;

	bool useBrackets = !bracketFilename.empty();
	SalaryBrackets brackets(useBrackets);

	if(useBrackets)
	{
		ifstream bracketFile(bracketFilename.c_str());
		brackets.initialize(bracketFile);
		bracketFile.close();
	}

	for(double overall = 65; overall <= 90; overall+=2)
	{
		outputFile << overall << "\t";
		for(double currentSalary = 400000; currentSalary<9000000; currentSalary+=400000)
		{
			if(useBrackets)
			{
				outputNewSalaryInfo(currentSalary,overall,outputFile,brackets);
			}
			else
			{
				outputNewSalaryInfo(currentSalary,overall,outputFile);
			}
			outputFile << "\t";
		}
		outputFile << endl;
	}

	outputFile << "RFA List:" << endl;

	for(size_t player = 0; player < nplayers; player++)
	{
		double age = players[player].getAge(2012,7,1);
		bool isRFA = players[player].getContractLength() == 1 && age < 31;
		if(isRFA)
		{
			const Player & currPlayer = players[player];
			double overall = rater.getOverall(currPlayer);
			if(overall > 65)
			{
				if(currPlayer.getLastName().compare("Backes") == 0)
				{
					int x = 5;
					x+=2;
				}

				double off = rater.getOFF(currPlayer);
				double def = rater.getDEF(currPlayer);
				double specialistBonus = 500000*(off >= (def+5)) + 250000*(def >= (off+5));

				if(useBrackets)
				{
					outputNewSalaryInfo(currPlayer, overall, outputFile, brackets, specialistBonus);
				}
				else
				{
					outputNewSalaryInfo(currPlayer, overall, outputFile);
				}
			}
		}
	}
}

double getCapLine(ifstream & teamCapFile, int day, int month, int year, Player * playerCaps[], int npcs,
	const vector<Player> & players, int pcs, ofstream & checkFile)
{
	if(teamCapFile.is_open() && !(teamCapFile.eof() || teamCapFile.peek() == EOF))
	{
		int iDay;
		teamCapFile >> iDay;
		if(iDay != day)
		{
			assert(iDay == day);
		}
		int iMonth;
		teamCapFile >> iMonth;
		assert(iMonth == month);
		int iYear;
		teamCapFile >> iYear;
		assert(iYear == year);

		caphit teamcap;
		teamCapFile >> teamcap;

		caphit penalties;
		teamCapFile >> penalties;

		caphit ltir;
		teamCapFile >> ltir;

		int nplayers;
		teamCapFile >> nplayers;

		for(int p = 0; p < nplayers; p++)
		{
			int id; teamCapFile >> id;
			int team; teamCapFile >> team;
			string firstName; teamCapFile >> firstName;
			string lastName; teamCapFile >> lastName;
			double salary; teamCapFile >> salary;

			int oldsal = 0;
			int currsal = 0;
			if(id >= 0)
			{
				if(id < npcs) oldsal = playerCaps[id]->getSalary();
				if(id < pcs) currsal = players[id].getSalary();
			}

			bool matchedSalary = (oldsal == salary || currsal == salary);
			if(!matchedSalary)
			{
				checkFile << "Mismatch game " << iDay << "/" << iMonth <<
					"/" << iYear << " playerid " << id << " salary " <<
					salary	<< " old file " << oldsal << " current file " <<
					currsal << endl;
			}
		}

		string temp;
		getline(teamCapFile,temp);

		if(!temp.empty())
		{
			throw runtime_error("Error! Team cap file EOL is " + temp +
				" instead of empty; aborting.");
		}

		return teamcap;
	}
	return 0;
}

double getAdjustedCap(double caphit, double penalties, double ltir, double maxcap)
{
	caphit = caphit + penalties;
	if(caphit > maxcap)
	{
		caphit = max(maxcap,caphit-ltir);
	}
	return caphit;
}

bool isPro(size_t team)
{
	return team > 0 && team <= 2*NTEAMS;
}

bool isNHL(size_t team)
{
	return team > 0 && team <= NTEAMS;
}

bool isAHL(size_t team)
{
	return team > NTEAMS && team <= 2*NTEAMS;
}

bool isPro(const Player & p)
{
	return(isPro(p.getTeam()));
}

bool isNHL(const Player & p)
{
	return(isNHL(p.getTeam()));
}

bool isAHL(const Player & p)
{
	return(isAHL(p.getTeam()));
}


caphit getPlayerCapHit(const Player & player, int year, int month, int day)
{
	bool nhl = isNHL(player);
	bool ahl = isAHL(player);
	return max(caphit(0), player.getSalary()*(nhl ||
		(ahl && (player.getAge(YEAR_FIRST, 9, 15) >= WAIVERAGE)))
		- MAXAHLSALARY*ahl);
}

pair<caphit, size_t> getCapHits(const vector<Player*> & capPlayers, caphit penalty, int year, int month, int day)
{
	vector<Player*>::const_iterator it;
	caphit cap = 0;
	size_t npro = 0;
	size_t ncon = 0;
	for (it=capPlayers.begin(); it!=capPlayers.end(); ++it)
	{
		const Player & p = **it;
		const size_t pteam = p.getTeam();
		//string lname = (**it).getLastName();
		if(pteam <= 2*NTEAMS)
		{
			cap += getPlayerCapHit(p, year, month, day);
			ncon += isPro(pteam);
		}
		npro += isNHL(pteam);
	}
	cout << npro << " " << (npro < MINNPRO) << " " << cap << endl;
	if(npro < MINNPRO) cap += (MINNPRO - npro)*MINCAPHITCURR;
	return {cap + penalty, ncon};
}

void writeCapLines(vector<int> gameDays, vector<int> gameMonths, vector<int> gameYears,
		vector<int> capTeams, caphit caphits[NTEAMS], size_t ncontracts[NTEAMS],
		ofstream teamOFiles[NTEAMS], vector<Player*> capPlayers[NTEAMS],
		caphit penalties[NTEAMS], caphit ltir[NTEAMS])
{
	unsigned int entries = capTeams.size();
	assert(entries == gameDays.size());
	assert(entries == gameMonths.size());
	assert(entries == gameYears.size());
	for(unsigned int entry = 0; entry < entries; entry++)
	{
		int team = capTeams.at(entry);
		ofstream & ofile = teamOFiles[team];
		ofile << gameDays.at(entry) << " ";
		ofile << gameMonths.at(entry) << " ";
		ofile << gameYears.at(entry) << " ";
		ofile << caphits[team] << " ";
		ofile << penalties[team] << " ";
		ofile << ltir[team] << " ";
		ofile << capPlayers[team].size();

		vector<Player*>::const_iterator it;

		for (it=capPlayers[team].begin(); it!=capPlayers[team].end(); ++it)
		{
			const Player & p = **it;
			string lastname = p.getLastName();
			bool done = false;
			while(!done)
			{
				size_t position = lastname.find(" ");
				if(position != string::npos)
				{
					lastname.replace(position,1,SPACEREPLACE);
				}
				else
				{
					done = true;
				}
			}
			ofile << " " << p.getId()  << " " << p.getTeam() << " " <<
				p.getFirstName() << " " << lastname << " " <<
				getPlayerCapHit(p, gameYears.at(entry),
				gameMonths.at(entry), gameDays.at(entry));
		}

		ofile << endl;
	}
}

double getRunningCap(ifstream & teamCapFile, int gamesPlayed, std::string teamFilename)
{
	double totalcap = 0;
	int gamesCounted = 0;
	if(teamCapFile.is_open())
	{
		bool keepReading = teamCapFile.peek() != EOF;
		while(keepReading)
		{
			int iDay;
			teamCapFile >> iDay;
			int iMonth;
			teamCapFile >> iMonth;
			int iYear;
			teamCapFile >> iYear;

			double caphit;
			teamCapFile >> caphit;
			assert(caphit > 0);

			double penalties;
			teamCapFile >> penalties;

			double ltir;
			teamCapFile >> ltir;

			caphit = getAdjustedCap(caphit, penalties, ltir, MAXCAP);

			totalcap += caphit;

			string restLine;
			getline(teamCapFile,restLine);

			gamesCounted++;

			keepReading = teamCapFile.peek() != EOF;
		}
	}
	if(gamesCounted != gamesPlayed)
	{
		throw runtime_error("Error! Games counted " + to_string(gamesCounted) +
			" doesn't match " + to_string(gamesPlayed) + "games played in file " +
			teamFilename + "; aborting.");
	}
	return totalcap;
}

void calcSalariesFromSchedule(string savedir, string capdirectory, int nteams,
	vector<Player*> capPlayers[NTEAMS], Player * playerCaps[], int npcs,
	const std::vector<Player> & players, int pcs, caphit penalties[NTEAMS], caphit ltir[NTEAMS])
{
	string scheduleFile = savedir + "/schedule.ehm";
	ifstream sched(scheduleFile);

	string leagueFile = savedir + "/league.ehm";
	ifstream league(leagueFile);

	vector<int> gameMonths;
	vector<int> gameDays;
	vector<int> gameYears;
	vector<int> capTeams;

	int gamesPlayed[NTEAMS];
	caphit caphits[NTEAMS];
	ifstream teamFiles[NTEAMS];
	string teamFilenames[NTEAMS];
	size_t ncontracts[NTEAMS];
	for(size_t i = 0; i < NTEAMS; i++)
	{
		gamesPlayed[i] = 0;
		caphits[i] = 0;
		ncontracts[i] = 0;
		teamFilenames[i] = capdirectory + "/" + TEAMNAMES[i] + ".txt";
		teamFiles[i].open(teamFilenames[i].c_str());
		bool success = teamFiles[i].is_open();
		if(!success) teamFiles[i].close();
	}

	bool keepReading = sched.is_open();

	int currDay = 0;
	int currMonth = 0;
	int currYear = 0;

	league >> currYear;
	league >> currMonth;
	league >> currDay;

	ofstream checkFile((capdirectory + "/check_caps.txt").c_str());
	checkFile.setf(ios::fixed);
	checkFile.precision(0);

	while(keepReading)
	{
		int gameDay;
		sched >> gameDay;
		int gameMonth;
		sched >> gameMonth;
		int gameYear;
		sched >> gameYear;
		size_t homeTeam;
		sched >> homeTeam; homeTeam--;
		if(homeTeam >= 0) assert(homeTeam < NTEAMS);
		size_t awayTeam;
		sched >> awayTeam; awayTeam--;
		if(awayTeam >= 0) assert(awayTeam < NTEAMS);
		int gameStatus;
		sched >> gameStatus;
		string temp;
		getline(sched,temp);
		// don't care about the score
		getline(sched,temp);

		bool stillRead = gameYear < currYear;
		if(gameYear == currYear)
		{
			stillRead = (gameMonth < currMonth) && (gameMonth > 0);
			if(gameMonth == currMonth)
			{
				stillRead = gameDay <= currDay;
			}
		}

		if(stillRead)
		{
			gamesPlayed[homeTeam]++;
			gamesPlayed[awayTeam]++;

			bool homeGameLogged = false;
			bool awayGameLogged = false;

			caphit homeCap = getCapLine(teamFiles[homeTeam],gameDay,gameMonth,gameYear, playerCaps, npcs,
					players, pcs, checkFile);
			assert(homeCap >= 0);
			homeGameLogged = homeCap > 0;
			caphit awayCap = getCapLine(teamFiles[awayTeam],gameDay,gameMonth,gameYear, playerCaps, npcs,
					players, pcs, checkFile);
			assert(awayCap >= 0);
			awayGameLogged = awayCap > 0;

			if(homeGameLogged != awayGameLogged)
			{
				throw std::runtime_error("Error! Home and away games not both logged; aborting.");
			}

			if(!homeGameLogged)
			{
				gameDays.push_back(gameDay);
				gameMonths.push_back(gameMonth);
				gameYears.push_back(gameYear);
				capTeams.push_back(homeTeam);
			}

			if(!awayGameLogged)
			{
				gameDays.push_back(gameDay);
				gameMonths.push_back(gameMonth);
				gameYears.push_back(gameYear);
				capTeams.push_back(awayTeam);
			}
		}

		keepReading = !sched.eof() && stillRead;
	}

	checkFile.close();

	for(size_t i = 0; i < NTEAMS; i++)
	{
		// Tally up penalties and LTIR separately
		auto rv = getCapHits(capPlayers[i],0,currYear,currMonth,currDay);
		caphits[i] = rv.first;
		ncontracts[i] = rv.second;
	}

	for(size_t i = 0; i < NTEAMS; i++) teamFiles[i].close();

	if(!capTeams.empty())
	{
		ofstream teamOFiles[NTEAMS];
		for(size_t i = 0; i < NTEAMS; i++)
		{
			teamOFiles[i].open((teamFilenames[i]).c_str(),std::ofstream::app);
			teamOFiles[i].precision(0);
			teamOFiles[i].setf(ios::fixed);
		}

		writeCapLines(gameDays,gameMonths,gameYears,capTeams,caphits,ncontracts,
			teamOFiles,capPlayers,penalties,ltir);

		for(size_t i = 0; i < NTEAMS; i++)
		{
			teamOFiles[i].close();
		}
	}

	double runningCaps[NTEAMS];
	for(size_t i = 0; i < NTEAMS; i++)
	{
		runningCaps[i] = 0;
		teamFiles[i].open((teamFilenames[i]).c_str());
	}

	ofstream capFile(capdirectory + "/caphits.txt");
	capFile.setf(ios::fixed);
	capFile.precision(0);

	char buf[1000];
	capFile << "TEAM  TEAMID  GP  TODAY     TODATE    PENALTIES LTIR      PROJECTED OVER_CAP CONTR  MAXCAP    CAPSPACE" << endl;

	for(size_t team = 0; team < NTEAMS; team++)
	{
		double caphit = getAdjustedCap(caphits[team], penalties[team], ltir[team], MAXCAP);
		double todate = getRunningCap(teamFiles[team], gamesPlayed[team], teamFilenames[team]);
		if(gamesPlayed[team] > 0) todate /= gamesPlayed[team];
		double projected = (todate*gamesPlayed[team] + caphit * double(NGAMES - gamesPlayed[team]))/double(NGAMES);
		std::string over = projected > MAXCAP ? "Y" : "N";
		double maxcap = (MAXCAP*NGAMES - todate*gamesPlayed[team])/double(NGAMES - gamesPlayed[team]);
		double capspace = maxcap-caphit;

		sprintf(buf, "%-6s%-6i%-6i%-10i%-10i%-10i%-10i%-10i  %-6s %-6i %-10i %-i",
			TEAMNAMES[team].c_str(),team+1, gamesPlayed[team],
			int(caphit), int(todate), penalties[team], ltir[team],
			int(projected), over.c_str(), ncontracts[team], int(maxcap), int(capspace));
		capFile << buf << endl;
	}

	capFile.close();
}

void readPenalties(char* filename, caphit * out)
{
	ifstream file;
	file.open(filename);
	std::string teamname;
	std::string line;
	for(size_t i = 0; i < NTEAMS; i++)
	{
		double total = 0;
		std::getline(file, line);
		std::stringstream ss(line);
		ss >> teamname;
		if(teamname != TEAMNAMES[i])
		{
			throw std::runtime_error("Error! Team " +
				teamname + "!=team[" + to_string(i) +
				"]=" + TEAMNAMES[i] + " in file " +
				filename + "; aborting.");
		}
		double penalty;
		while(ss >> penalty)
		{
			total += penalty;
		}
		out[i] = total;
	}
	file.close();
}

int main(int argc, char * argv[])
{
	if(argc < 3 || argc > 12)
	{
		cout << "Error! Must have minimum 2 input arguments: " << endl
				<< " 1. Input file and 2. output file" << endl
				<< " And maximum 6. Options are:" << endl
				<< "3. Read CSV/Write EHM, 4. Start of season input file" << endl
				<< "5. salary cap output directory, 6. optional cap penalty file" << endl
				<< "(Cap penalties must be 30 line file in same order as teams file)" << endl;
		cout << "7. LTIR file 8. Save directory " << endl;
		exit(EXIT_FAILURE);
	}

	const int tempdataSize = 1024;
	char tempdata[tempdataSize];

	ifstream inputFile;
	inputFile.open(argv[1]);

	ofstream outputFile;
	outputFile.open(argv[2]);

	const string CSV = argv[3];
	const bool isCSV = (CSV == "1" || CSV == "T" || CSV == "true" || CSV == "True");

	size_t nplayers = 0;

	std::vector<Player> players;
	if(!isCSV)
	{
		inputFile >> nplayers;
		players.reserve(nplayers);
	}

	if(isCSV)
	{
		outputFile << "         " << std::endl;
		int next = inputFile.peek();
		while(next != EOF)
		{
			players.push_back(Player(inputFile, !isCSV, tempdata, tempdataSize, nplayers));
			players[nplayers].outputDataEHM(outputFile);
			nplayers++;
			next = inputFile.peek();
		}
		outputFile.seekp(0, ios_base::beg);
		outputFile << " " << nplayers << " ";
	}
	else
	{
		outputFile << "\"sh\",\"pl\",\"st\",\"ch\",\"po\",\"hi\",\"sk\",\"en\",\"pe\","
					"\"fa\",\"le\",\"str\",\"pot\",\"con\",\"gre\",\"fi\",\"click\",\"team\","
					"\"position\",\"country\",\"hand\",\"byear\",\"bday\",\"bmonth\",\"salary\","
					"\"years\",\"draft_year\",\"draft_round\",\"draft_team\",\"rights\",\"thisweek_gp\","
					"\"thisweek_g\",\"thisweek_a\",\"thisweek_gwg\",\"thismonth_gp\",\"thismonth_g\","
					"\"thismonth_a\",\"thismonth_gwg\",\"records_g\",\"records_a\",\"records_p\","
					"\"notrade\",\"twoway\",\"option\",\"status\",\"rookie\",\"offer_status\","
					"\"offer_team\",\"offer_time\",\"injury_status\",\"scout_1_10\","
					"\"scout_11_20\",\"scout_21_30\",\"streak_g\",\"streak_p\",\"gp\",\"suspension\","
					"\"training\",\"weight\",\"height\",\"status_org\",\"streak_best_gp\","
					"\"streak_best_gwg\",\"streak_best_p\",\"streak_best_a\",\"streak_best_g\","
					"\"unused\",\"name_first\",\"name_last\",\"performance\",\"acquired\",\"ceil_fi\","
					"\"ceil_sh\",\"ceil_pl\",\"ceil_st\",\"ceil_ch\",\"ceil_po\",\"ceil_hi\","
					"\"ceil_sk\",\"ceil_en\",\"ceil_pe\",\"ceil_fa\",\"ceil_le\",\"ceil_str\","
					"\"version\",\"attitude\",\"position_alt\",\"rights_2\","
					"\"injury_prone\",\"draft_overall\",\"id\"" << std::endl;
		for(uint i = 0; i < nplayers; i++)
		{
			players.push_back(Player(inputFile, !isCSV, tempdata, tempdataSize, i));
			players[i].outputDataCSV(outputFile, i);
		}
	}

	inputFile.close();

	try
	{
		if(argc > 4)
		{
			ifstream capFile;
			capFile.open(argv[4]);

			Player * playerCaps[nplayers];

			vector<Player*> capPlayers[NTEAMS];

			for(int i = 0; i < nplayers; i++)
			{
				playerCaps[i] = new Player(capFile, true, tempdata, tempdataSize,i);

				const Player & p = *(playerCaps[i]);
				int team = p.getRights();
				bool ahl = team > NTEAMS && team <= 2*NTEAMS;

				// Add AHL players too
				if((((team > 0) && (team <= NTEAMS)) || ahl) && p.getContractLength() > 0)
				{
					capPlayers[team-1-ahl*NTEAMS].push_back(playerCaps[i]);
					/*
					if(players[i]->getContractLength() == 0)
					{
						capPlayers[team-1].push_back(players[i]);
					}
					else
					{
						capPlayers[team-1].push_back(playerCaps[i]);
					}
					*/
				}
			}

			capFile.close();
			vector<Player*>::const_iterator it;

			ofstream capOutfile;
			string capdir = string(argv[5]);

			if(!(::GetFileAttributesA(capdir.c_str()) && FILE_ATTRIBUTE_DIRECTORY))
			{
				cerr << "Error! Directory " << capdir << " does not exist; please create it first." << endl;
				exit(EXIT_FAILURE);
			}

			caphit penalties[NTEAMS];

			if(argc > 6)
			{
				readPenalties(argv[6], penalties);
			}
			else
			{
				for(int i = 0; i < NTEAMS; i++) penalties[i] = 0;
			}

			caphit ltir[NTEAMS];

			if(argc > 7)
			{
				readPenalties(argv[7], ltir);
			}
			else
			{
				for(size_t i = 0; i < NTEAMS; i++) ltir[i] = 0;
			}

			if(argc > 8)
			{
				string savedir = argv[8];
				calcSalariesFromSchedule(savedir, capdir, NTEAMS, capPlayers, playerCaps, nplayers,
						players, nplayers, penalties, ltir);
				capOutfile.open((capdir + "/" + "caps.txt").c_str());

				string leagueFile = savedir + "/league.ehm";
				ifstream league(leagueFile);

				int year; int month; int day;
				league >> year;
				league >> month;
				league >> day;

				for(size_t i = 0; i < NTEAMS; i++)
				{
					sort(capPlayers[i].begin(), capPlayers[i].end(), higherSalary);
					capOutfile << TEAMNAMES[i] << endl;
					int playersCounted = 0;

					auto caps = getCapHits(capPlayers[i], penalties[i],
						year, month, day);
					caphit teamcap = caps.first;
					caphit tcaprun = 0;

					for (it=capPlayers[i].begin(); it!=capPlayers[i].end(); ++it)
					{
						const Player & p = (**it);
						caphit pcaphit = getPlayerCapHit(**it, year, month, day);
						capOutfile << p.getLastName() << ", " << p.getFirstName() << "\t" <<
						pcaphit << endl;
						tcaprun += pcaphit;
						playersCounted++;
						/*
						if(playersCounted == 23)
						{
							break;
						}*/
					}
					capOutfile << "Roster: " << tcaprun << endl;
					capOutfile << "Penalties: " << penalties[i] << endl;
					capOutfile << "Total: " << tcaprun+penalties[i] <<
						" vs. final " << teamcap << endl;
					capOutfile << "Contracts: " << caps.second << endl << endl;
				}

				capOutfile.close();
			}

			if(argc > 9)
			{
				string salaryFilename = argv[9];
				string overallFilename = argv[10];

				if(argc > 11)
				{
					outputNewSalariesInfo(players, nplayers, salaryFilename, overallFilename, string(argv[11]));
				}
				else
				{
					outputNewSalariesInfo(players, nplayers, salaryFilename, overallFilename);
				}
			}
		}
	}
	catch(exception & e)
	{
		cerr << "Caught exception: " << e.what() << endl;
	}

	return EXIT_SUCCESS;
}
