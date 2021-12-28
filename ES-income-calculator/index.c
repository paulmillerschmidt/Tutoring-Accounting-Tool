#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <stdbool.h>

#define NAME_LENGTH 20
#define TRANSACTION_FEE_PERCENTAGE 2.9
#define FEE_PER_CREDIT_CARD .30
#define PERCENT_DONATION_PER_ORGANIZATION 2.5
#define NUM_ORGANIZATIONS 2
#define CURRENT_MANAGEMENT_TEAM_SIZE 1
#define MANAGEMENT_FEE_PERCENTAGE 10
#define NUMBER_OF_PRICING_PLANS 5

typedef struct tutor {
    char *name;
    int numStudents;
    struct student *students;
} tutor;

typedef struct student {
    char *name;
    double weeklyCost;
    int numPaymentsProcessed;
    bool biweekly;
} student;

bool approximateSum(double *addends, double addend, double sum, int numTutors) {
    int numNonZeroAddends = 0;
    for (int i = 0; i < numTutors; i++) {
        if (addends[i] != 0) {
            numNonZeroAddends++;
        }
    }
    double sumAddends = 0.0;
    for (int i = 0; i < numNonZeroAddends; i++) {
        sumAddends += addends[i];
    }
    sumAddends += addend;
    return (fabs(sumAddends - sum) <= .05);
}

int tutorSum(tutor tutor, int numStudents) {
    int sum = 0;
    for (int i = 0; i < numStudents; i++) {
        sum += (tutor.students[i].weeklyCost * tutor.students[i].numPaymentsProcessed);
    }
    return sum;
}

int teamSum(tutor *tutors, int numTutors) {
    int teamSum = 0;
    for (int i = 0; i < numTutors; i++) {
        teamSum += tutorSum(tutors[i], tutors[i].numStudents);
    }
    return teamSum;
}

int numHours(tutor tutor) {
    int numHours = 0;
    for (int i = 0; i < tutor.numStudents; i++){
        int numPayments = tutor.students[i].numPaymentsProcessed;
        numHours += (tutor.students[i].biweekly ? numPayments * 2 : numPayments);
    }
    return numHours;
}

int sumHours(tutor *tutors, int numTutors) {
    int sumHours = 0;
    for (int i = 0; i < numTutors; i++) {
        sumHours += numHours(tutors[i]);
    }
    return sumHours;
}

double teamSumAfterTransactionFees(tutor *tutors, int numTutors) {
    double sum = (double) teamSum(tutors, numTutors);
    int numClients = 0;
    for (int i = 0; i < numTutors; i++) {
        numClients += tutors[i].numStudents;
    }
    sum -= (sum * (TRANSACTION_FEE_PERCENTAGE / 100));
    double teamSumAfterTransactionFees = sum - (FEE_PER_CREDIT_CARD * numClients);
    return teamSumAfterTransactionFees;
}

double teamSumAfterDonations(tutor *tutors, int numTutors) {
    double sumAfterFees = teamSumAfterTransactionFees(tutors, numTutors);
    return sumAfterFees * (100 - (NUM_ORGANIZATIONS * PERCENT_DONATION_PER_ORGANIZATION)) / 100;
}

double donationPerOrganization (tutor *tutors, int numTutors) {
    double sumAfterFees = teamSumAfterTransactionFees(tutors, numTutors);
    return sumAfterFees * PERCENT_DONATION_PER_ORGANIZATION / 100;
}

double *grossPayouts(tutor *tutors, int numTutors) {
    double *grossPayouts = (double *) malloc (numTutors * sizeof(double));
    for (int i = 0; i < numTutors; i++) {
        double unroundedPayout = teamSumAfterDonations(tutors, numTutors) * numHours(tutors[i]) / sumHours(tutors, numTutors);
        grossPayouts[i] = round(unroundedPayout * 100) / 100;
    }
    return grossPayouts;
}

bool includes(char **names, char *name, int numNames) {
    for (int i = 0; i < numNames; i++) {
        if (strcasecmp(names[i], name) == 0){
            return true;
        }
    }
    return false;
}

char **getOrganizations() {
    char **organizations = (char **) malloc (NUM_ORGANIZATIONS * sizeof(char *));
    for (int i = 0; i < NUM_ORGANIZATIONS; i++) {
        organizations[i] = (char *) malloc (NAME_LENGTH * 2 * sizeof(char));
    }
    organizations[0] = "No Kid Hungry";
    organizations[1] = "NAACP Legal Defense Fund";
    return organizations;
}

char **getManagementTeam() {
    char **management = (char **) malloc (CURRENT_MANAGEMENT_TEAM_SIZE * sizeof(char *));
    for (int i = 0; i < CURRENT_MANAGEMENT_TEAM_SIZE; i++) {
        management[i] = (char *) malloc (NAME_LENGTH * sizeof(char));
    }
    management[0] = "Jarett";
    return management;
}

double managementDividend(tutor *tutors, int numTutors) {
    double *gross = grossPayouts(tutors, numTutors);
    double dividend = 0.0;
    for (int i = 0; i < numTutors; i++) {
        if (!includes(getManagementTeam(), tutors[i].name, CURRENT_MANAGEMENT_TEAM_SIZE)) {
            dividend += gross[i] * MANAGEMENT_FEE_PERCENTAGE / 100;
        }
    }
    return dividend;
}

double *netPayouts(tutor *tutors, int numTutors) {
    int internalCount = 0;
    double *gross = grossPayouts(tutors, numTutors);
    double *net = (double *) malloc (numTutors * sizeof(double));
    double individualManagementDividend = managementDividend(tutors, numTutors) / CURRENT_MANAGEMENT_TEAM_SIZE;
    for (int i = 0; i < numTutors; i++) {
        if (includes(getManagementTeam(), tutors[i].name, CURRENT_MANAGEMENT_TEAM_SIZE)) {
            internalCount++;
            net[i] = gross[i] + individualManagementDividend;
        }
        else {
            net[i] = gross[i] * (100 - MANAGEMENT_FEE_PERCENTAGE) / 100;
        }
    }
    if (internalCount != CURRENT_MANAGEMENT_TEAM_SIZE) {
        printf("ERROR: Management team size incorrect.\n");
        printf("\tCheck for proper spelling of tutors' names.\n");
    }
    return net;
}


static bool isValidPricingPlan(char *plan) {
    char *plans[NUMBER_OF_PRICING_PLANS] = {"ms", "hsPrep", "hs", "collPrep", "hsSplit"};
    return includes(plans, plan, NUMBER_OF_PRICING_PLANS);
}

static double planToWeeklyCost(char *plan, bool biweekly) {
    double cost;
    if (strcasecmp(plan, "ms") == 0) {
        cost = 39;
    }
    else if (strcasecmp(plan, "hsPrep") == 0) {
        cost = 49;
    }
    else if (strcasecmp(plan, "hs") == 0) {
        cost = 59;
    }
    else if (strcasecmp(plan, "collPrep") == 0) {
        cost = 79;
    }
    else if (strcasecmp(plan, "hsSplit") == 0) {
        cost = 109 / 2;
    }
    else {
        cost = -1;
    }
    if (biweekly) {
        cost = (cost * 2) - 9;
    }
    return cost;
}


void main()
{
    int numTutors, numStudents, numWeeklySessions;
    char *name;
    printf("How many tutors worked this week? ");
    scanf("%d", &numTutors);
    tutor *tutors = (struct tutor *) malloc (numTutors * (sizeof (struct tutor)));
    int *tutorSums = (int *) malloc (numTutors * sizeof(int));

    
    for (int i = 0; i < numTutors; i ++)
    {
        printf("First name of Tutor %d: ", i + 1);
        tutors[i].name = (char *) malloc (NAME_LENGTH * sizeof(char));
        scanf("%s", tutors[i].name);
        printf("How many students did %s tutor this cycle? ", tutors[i].name);
        scanf("%d", &tutors[i].numStudents);
        student *students = (struct student *) malloc (tutors[i].numStudents * (sizeof (struct student)));
        for (int j = 0; j < tutors[i].numStudents; j++) {
            char *plan = (char *)malloc(sizeof(char) * 15);
            printf("First name of %s student #%d: ", tutors[i].name, j + 1);
            students[j].name = (char *)malloc(NAME_LENGTH * sizeof(char));
            scanf("%s", students[j].name);
            while (!isValidPricingPlan(plan)) {
                printf("%s's pricing plan (ms/hsPrep/hs/collPrep): ", students[j].name);
                scanf("%s", plan);
            }
            printf("Number of %s's payments processed this cycle: ", students[j].name);
            scanf("%d", &students[j].numPaymentsProcessed);
            printf("Number of %s's hour-long sessions per week: ", students[j].name);
            scanf("%d", &numWeeklySessions);
            students[j].biweekly = (numWeeklySessions == 2);
            double cost = planToWeeklyCost(plan, students[j].biweekly);
            if (cost > 0) {
                students[j].weeklyCost = cost;
            }
            else {
                printf("ERROR: Invalid weekly cost!");
            }
        }
        tutors[i].students = students;
    }
    
    double *net = netPayouts(tutors, numTutors);
    double teamSumPostFees = teamSumAfterTransactionFees(tutors, numTutors);
    double donation = donationPerOrganization(tutors, numTutors);
    printf("–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––\n");
    
    if (approximateSum(net, donation * NUM_ORGANIZATIONS, teamSumPostFees, numTutors)) {
        printf("NET PAYOUTS:\n");
        for (int i = 0; i < numTutors; i++) {
            printf("\t%s: $%.2lf\n", tutors[i].name, net[i]);
        }
        
        char **organizations = getOrganizations();
        printf("DONATIONS:\n");
        for (int i = 0; i < NUM_ORGANIZATIONS; i++) {
            printf("\t%s: $%.2lf (%.1lf%s donation)\n", organizations[i], donation, PERCENT_DONATION_PER_ORGANIZATION, "%");
        }
    }
    else {
        printf("There has been a math error...");
    }
}





