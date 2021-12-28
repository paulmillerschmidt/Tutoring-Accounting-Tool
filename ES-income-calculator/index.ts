const transactionFeePercentage = 2.9;
const creditCardFee = .30;
const percentDonationPerOrganization = 2.5;
const organizations = ['no kid hungry', 'naacp'];
const managementTeam = ['jarett'];
const managementFeePercentage = 10;
const pricingPlans: PlanTitle[] = ['middle school', 'high school prep', 'high school', 'college prep'];

type PlanTitle = 'middle school' | 'high school prep' | 'high school' | 'college prep';

interface Tutor {
  name: string,
  students: Student[],
  management?: boolean,
}

interface Student {
  name: string,
  pricingPlan: PricingPlan,
  paymentsProcessed: number,
}

interface PricingPlan {
  planTitle: PlanTitle,
  biweekly?: boolean,
  split?: boolean;
}

interface Payout {
  [tutor: string]: number
}

const planToWeeklyPriceMap = {
  'middle school': 39,
  'high school prep': 49,
  'high school': 59,
  'college prep': 79
}

function isApproximatelyRight(addends: number[], addend: number, sum: number) : boolean {
  const sumAddends = addends.reduce((acc, val) => acc + val, 0) + addend;
  return Math.abs(sumAddends - sum) <= .05;

}

function getTutorSum(tutor: Tutor): number {
  return tutor.students.reduce((acc, student) => acc + getWeeklyPrice(student.pricingPlan) * student.paymentsProcessed, 0);
}

function getTeamSum(tutors: Tutor[]): number {
  return tutors.reduce((acc, tutor) => acc + getTutorSum(tutor), 0);
}

function getTutorHours(tutor: Tutor): number {
  return tutor.students.reduce((acc, student) => acc + student.paymentsProcessed * (student.pricingPlan.biweekly ? 2 : 1) * (student.pricingPlan.split ? 0.5 : 1), 0);
}

function getTeamHours(tutors: Tutor[]): number {
  return tutors.reduce((acc, tutor) => acc + getTutorHours(tutor), 0);
}

function getTeamSumAfterTransactionFee(tutors: Tutor[]): number {
  const numberOfClients = tutors.reduce((acc, tutor) => acc + tutor.students.length, 0);
  const totalSum = getTeamSum(tutors);
  const transactionFees = totalSum * transactionFeePercentage / 100;
  const creditFees = numberOfClients * creditCardFee;
  return totalSum - (transactionFees + creditFees);
}

function getTeamSumAfterDonations(tutors: Tutor[]): number {
  let totalSum = getTeamSumAfterTransactionFee(tutors);
  totalSum -= getTotalDonation(tutors);
  return totalSum;
}

function getDonationPerOrganization(tutors: Tutor[]): number {
  const totalSum = getTeamSumAfterTransactionFee(tutors);
  return totalSum * percentDonationPerOrganization / 100;
}

function getTotalDonation(tutors: Tutor[]): number {
  const donationPerOrganization = getDonationPerOrganization(tutors);
  return donationPerOrganization * organizations.length;
}

function getGrossPayout(tutors: Tutor[]): Payout {
  const payout: Payout = {};
  const teamSum = getTeamSumAfterDonations(tutors);
  const teamHours = getTeamHours(tutors);
  for (let tutor of tutors) {
    payout[tutor.name] = round(teamSum * getTutorHours(tutor) / teamHours)
  }
  return payout;
}

function round(i: number): number {
  return Math.round(i * 100) / 100;
}

function getManagementDividend(tutors: Tutor[]): number {
  const grossPayout = getGrossPayout(tutors);
  let dividend = 0;
  for (let tutor of tutors) {
    if (!managementTeam.includes(tutor.name.toLowerCase())) {
      dividend += grossPayout[tutor.name] * managementFeePercentage / 100;
    }
  }
  return dividend;
}

function getNetPayout(tutors: Tutor[]): Payout {
  let net: Payout = {};
  const individualManagementDividend = getManagementDividend(tutors) / managementTeam.length;
  const gross = getGrossPayout(tutors);
  for (let tutor of tutors) {
    net[tutor.name] = gross[tutor.name];
    if (managementTeam.includes(tutor.name.toLowerCase())) {
      net[tutor.name] += individualManagementDividend;
    }
    else {
      net[tutor.name] *= (100 - managementFeePercentage) / 100;
    }
  }
  return net;
}

function getWeeklyPrice(plan: PricingPlan) {
  let price = planToWeeklyPriceMap[plan.planTitle];
  if (plan.biweekly) {
    price = price * 2 - 9;
  }
  if (plan.split) {
    price *= 0.5;
  }
  return price;
}

function getStudents(studentsPayments: {[name: string]: number}): Student[] {
  const studentArray: Student[] = [];
  for (let name of Object.keys(studentsPayments)) {
    if (students[name] && studentsPayments[name] > 0) {
      studentArray.push({...students[name], paymentsProcessed: studentsPayments[name]});
    }
  }
  return studentArray;
}

const students: {[studentName: string]: {name: string, pricingPlan: PricingPlan}} = {
  'Camille': {name: 'Camille Legaspi', pricingPlan: {planTitle: 'high school', biweekly: true, split: true}},
  'Ryan': {name: 'Ryan Pinsker', pricingPlan: {planTitle: 'high school'}},
  'Corey': {name: 'Corey Herman', pricingPlan: {planTitle: 'middle school'}},
  'Owen': {name: 'Owen Wheatley', pricingPlan: {planTitle: 'high school prep', biweekly: true}},
  'Andrew': {name: 'Andrew Jeon', pricingPlan: {planTitle: 'middle school'}},
  'Simon': {name: 'Simon Burkhardt', pricingPlan: {planTitle: 'middle school'}},
  'Isabelle': {name: 'Isabelle Lleras', pricingPlan: {planTitle: 'high school', biweekly: true}}
}

const eliteSuite: Tutor[] = [
  {name: 'Jarett', students: getStudents({'Camille': 3, 'Ryan': 2, 'Corey': 2}), management: true},
  {name: 'Paul', students: getStudents({'Camille': 1})},
  {name: 'Keon', students: getStudents({'Simon': 2, 'Isabelle': 0})},
  {name: 'Andrew', students: getStudents({'Andrew': 2})},
  ];

function printPayoutInfo(tutors: Tutor[]) {
  const net = getNetPayout(tutors);
  if (!isApproximatelyRight(Object.values(net), getTotalDonation(tutors), getTeamSumAfterTransactionFee(tutors))) {
    console.log("There has been a math error.");
    return;
  }
  demarcate("DONATIONS");
  const individualDonation = getDonationPerOrganization(tutors);
  for (let organization of organizations) {
    console.log(`${organization}: $${individualDonation.toFixed(2)}`);
  }
  demarcate("TUTOR PAYOUTS");
  for (let name of Object.keys(net)) {
    console.log(`${name}: $${net[name].toFixed(2)}`);
  }
}

function demarcate(s: string){
  let line = '-----------';
  console.log(line);
  console.log(s);
  console.log(line);
}

printPayoutInfo(eliteSuite);
