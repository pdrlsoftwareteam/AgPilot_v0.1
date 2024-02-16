/*
 * keystore.cpp
 *
 *  Created on: 31-Jul-2019
 *      Author: Owner
 */
#include "AP_KEYSTORE.h"
//#include <AP_LIBNPNT/AP_LIBNPNT.h>
#include <GCS_MAVLink/GCS.h>
#include "AP_PDRL_Commander_Logger.h"
#include "../../libraries/AP_HAL_ChibiOS/hwdef/common/stm32_util.h"

extern const AP_HAL::HAL& hal;
AP_KEYSTORE* AP_KEYSTORE::m_pInstance = 0;
//* Drone Acharya */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEApLgz87owBTew4NNc5OQQ\n9qs8qliYtKsH6OETdXKdABJt7vlasafeuvqierSDfhF0YxJhEQThznq6zb7Mr3mD\nril6Xfcdvx4z6BaByuG5ruXDWD80KzWnzzeEr2FFBi8t5dJRuC4AzDmTPxrbnsYd\n9EK1EC5C6oLsrpVZzCrqNJdC2ZCK2CRUW+6XS1qURR9lGLWR42BG3ixNq/1mKTyl\nDAOrOZPND09qlGoHfmMvHMl0wQcYc1s14hUhPkvVNaGj0Rxz91hVBZfU0zyWzLBL\nB5PiL3J93Awt6SpphqDVbZZJpqA3sjudnN4Ch+yNU3NYLY0rWDhofAjvrVHyQDa1\njQIDAQAB\n-----END PUBLIC KEY-----\n\0";
 //*Clarion*/ const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsFVqdDxO94WoNekVRh8W\n+jquKPfMvFP4vN52/ZU7xxD7ktteg2MK5wTN1m5qnb+3wDjEpFctWjey2JdDgjHO\nr0Udai9yvtIaB7g1/KyqjhQWWnGEBUR2vff5T0KkuwhCSDF/7URZklfPIZMImuKg\nKqtP0B6vfAGV+crhzL9yC2+EZCzZrnip48+SWdU7tTwp3qjrRhyhYhNYexp8Z2s9\nHB+hRPe/+D4Tl8uX1+5j5aKi58Vcyx7+34oewYEdo7RKFqtSzkgcaC8AmrRAMkxi\nhCivLTW7KTAhJIFOsblAXOQzR88AdGuOhqQBZ42WhGdiir9tCnGTanXCrSkmoag0\nrQIDAQAB\n-----END PUBLIC KEY-----\n\0";
//* Nextleap aeronautics */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAjWmLkH1jwodafZmZbRGq\nAMLYQtLyhs3EdL/hG0VLsoCFN5aBR66yNb9xeyE5bCk9wKZkwn8nCNAMmc30CZ8e\nTXw+20yhNY8RayrxO2YKy0Ymq/qe3HgdgTwUY504Yy54yHJJpQKw4pMxjrS10U4H\nfmq9vir4wZXHDVVgc3Ynupyjk1w8Rbifl/NodZGqkqdM2Jl3dOfQVqV7kE+JTZmv\nB6etTCRqrEUW2EBRBCcZfxMhZoEie7LrtE330pVnxIcNdLPXPGo8lF1g2JVqLfY/\nwegPavt+WEqVElRTa3UAvqseRfimtf3IYVTTrDYJViTdpHtfA9y1kACeUWoxK9TJ\nKwIDAQAB\n-----END PUBLIC KEY-----\n\0";
//* SkyX aerospace */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtJXFAgTBIjrC1BWf/FXK\narIaZVgJgrwDj8dVNxD+zghrPEvDYUYRRgKJ2awICyFw7PlsE91vjqMemYWMhqO5\nkQFLJdCQ+YjBJu4Hz7EBOaZ+w4fYRfFkaFdMKn6vNzFdQzmQECDYPZxCgKvABSBt\nPCwqyIXFKIceeBlkBnEo5Wj99WlsOP3SI9ovWWASZ/kutSBMLqiWDm91H9R8pAM2\nwklXCwwwqxNx5itABM+0oBZ1nCRXIMlrTyLCzA0Ceeaaa3AYVWT73K7Gm6COLD/5\nzgNO8/gI2Vyzdy8WFRoXgkg3AuxriAbDJKRnfrQGX/HesIbV4YnJqehPGzfbg3KX\nHQIDAQAB\n-----END PUBLIC KEY-----\n\0";
//* Aeroaeon_Avionics */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAm3Dnt5cDLM/DLApEO6eh\nKvV+FziNp7Qpy6+TGZ7cPu3s9JqZCbmEHOSJfNHYwumfaBXspPWeu4kMyRC9RGR/\n5gwGXKglqABnbWQAaPDhibs7UhwaNzeZVwlotlMDWP7tQK+T+Cpge+Ae0TvQ00wJ\nwK4CL1Bw9iV0WopByn7bgwk91IcNY07fApudqspTcmDVIhdTS2sa+OjRxftFbKHK\n/kKCCRwpArEEgYFT1XrTXgV3r+blE86zZQs01U7RXQCXl2d2CwzGT35vwdfb1LMv\nv0fKkr4aIiZKDF+49LQ5AHhkhGbBu5QyHN5qCa5kqtu5XM4amx8unIN9RNqSSYdw\nUQIDAQAB\n-----END PUBLIC KEY-----\n\0";
//* Vyomic */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2ube85LC6PqcLYxU1I4+\nwhStL9KVeZkhabSM29UH6sUmDxEmDHKddX7yMX0xxaAeHjKZl2YiszevR39WydKj\nDXDBpDkwqx+4/oeL9DuGsYJwKzAbSvOUIkQMoOLFnrkecQtqolJcJhef53MPsieB\nrIhBRolZXNWApzeu53G8RYzo9EHL7s3kyvHRF7v+BgGwTvFZVb20UwqgCQcivaQ/\n6EJx1q+I3NLwrqNNF/C6BvKAid3T+ZMiSXm+2x3EbPO0HjoufNJiYol8IYwD6+nU\nU1aqhlgFeSiiSMNt1gZAKkHAH+AqXj74eE9OgFuZIfw7ODvBAUMQNFYi70caOYzB\nEQIDAQAB\n-----END PUBLIC KEY-----\n\0";
//* Aeroarc */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAs00Lq3O72w7l4kfjc7JR\n1+PueUit0L5BexDr/McAhPTJz0x1obYUWukWin4CHPLswH225yNSK7pbWe1jzUCu\n/RR2VM2VkG3Ki8LkFIAAglY6f+x0dJLfwPNN3s/KW7SnCzsxcCqhMpc228pxFEkI\n56uesVDx80oypdGeFx+1q/YBGP2rvcJ0/g31wYh4WMQH5gVN5NKWstRwVj71NXFe\n8E3bZzostoj5GwFznumh+lv8WVo8JNy1oZdcVsxqTsPEUw9tmFEJqG36nlNepLFx\ntu7Auz+scZgJt+tQNAXcaDisjuLAHFg8lr+vQ4OAnAz9PMAO05NPy3YNEtphhlgb\nUwIDAQAB\n-----END PUBLIC KEY-----\n\0";
///* Indrone */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAgZiprbLFPlf/haCtsMHo\na+NYTrCvlONavZwtPMCbA1Uou8/QBLA5im3LvqHopouoKnpDMmBdpjSpIa4HgtR0\nkI0GsAgBtgVewUXHGf5mLw96LRudI/TPjay7kRoE0iLmOh0lHxM43duSWT4JuDIv\nDzWJi/cMm4RxNnqcrMiUQaZ3QurUMLPl4V2pnH+wY/f7ZK4UQ/o9cH/356aaINsD\nvYo4QoPrOhQmNMMmpdfxI2PGg0XHnyvMb8GKj0jLIZMfxBt2BqFPFxJlhnxtXs25\nmxa6iBTMBStoFFAwK0QFpbYkDsXwbYmp/un5RcbgqVTzisOQHyzsHte2tjsJJR+f\ngQIDAQAB\n-----END PUBLIC KEY-----\n\0";
//* Enercomp Solution */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA0wiYPgp5xBxVCx170cFM\n0h+SfTwnc7WrTz1uydBpcN+KgTZn1Ts6wdhIRApasJdIU2EllvI4wfVJ4hkDZqur\n/qXtwu9O7vcLluRCR6N2ThBEJt82EgLcLDHdRfp8f5PodvuHNCtnY+VFlil/6vNW\nft5dCZINOfZq4SNs55PxmXBhokNZ5XbKM2dach7aBgoeOThsrIT7c+8qEcXE9JOm\n1J4SW/Bw14s6dj7FNnxqJVkaJYxf3u7RUdZ+TG/tBoOl4+xHT5W8Fdj5wgggtGDx\n63GpOEvBwSRVahZvBNt5n8PLt2yFhwRQ2WGMQP4pog9VO821WHKjZU5yhGBGJiSB\n8wIDAQAB\n-----END PUBLIC KEY-----\n\0";
//* SCANDRON */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAifGDGUY5NnhPrjsC1VPo\nOnKnpWXeXrmvADQ6t3bbjeluOMcXRVVrjr1/bmebHQWHnhMMwQNlIjwPTRK4mTmI\nIevSaNU/KJi3a6L7mwl2G2S9xytXWVyB+bWMb04+fLE4ZoPRSlj3WhV0OO0WGtMn\nKYRvMpZ/PfJPQiRpUNJj7n31wsAoUYATYTmvv3L7kFm8pDFL73BlKsS9IqP9/6vF\nHZzvVYwHA9ohU98QRnpCl8ygq1mtz0aLAeYaJC1iqnWUqVTa3C8XqBgqpu5CZueF\n279jnfbKZAEKThWck8WZdm5DNfAoh8A5KDUgnO40qh/72zM8irm9As9lxCeNtiVP\nNQIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Generated Key Aniket*/const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2fPAsXTphWBfgbiPdLaI\nl9S7X4k1N2fKZY7M0mVFG98f+F7SliIJksS40ioftF7Xs6EpiHyJSP3RbZdwU8wb\ni1wkRTGDXeckl7j18S1lWgUtAPMBL+IAQbezyilnzoXVYyCVyriQpfDwoOLWGkQL\n1lyMrmU+gTXBNBd49uqfwTKlibp6gptR6xkezAVH+bMsrQrYWsD1Jb6X7nnpyDNR\nxUJXwtTvw+NbxNMg+wm5vfciYKpqGdjfSqVNHVr/NYxSr+FhvGdonyFwWI1z2PW5\nB4Dbe+5qlK/zXSCOHjSYqXScqg4YKpXJvwI+yTHZ/Rg3/bdvPYoqznz42ZFuBHJL\ncQIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Aniket   */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2fPAsXTphWBfgbiPdLaI\nl9S7X4k1N2fKZY7M0mVFG98f+F7SliIJksS40ioftF7Xs6EpiHyJSP3RbZdwU8wb\ni1wkRTGDXeckl7j18S1lWgUtAPMBL+IAQbezyilnzoXVYyCVyriQpfDwoOLWGkQL\n1lyMrmU+gTXBNBd49uqfwTKlibp6gptR6xkezAVH+bMsrQrYWsD1Jb6X7nnpyDNR\nxUJXwtTvw+NbxNMg+wm5vfciYKpqGdjfSqVNHVr/NYxSr+FhvGdonyFwWI1z2PW5\nB4Dbe+5qlK/zXSCOHjSYqXScqg4YKpXJvwI+yTHZ/Rg3/bdvPYoqznz42ZFuBHJL\ncQIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*PDRL     */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAksUNzNaLPLEsgQ9AmGG9\nDKdR2l3T0OdQ8kEXR8fLTrD+Bf1D0tFR/naEBqO1+dkPxlzeHOq50MjC5rFWjGLO\nfIrDDXdl1ODzg6LWzErCG9GkRy7ItEmYbfxsxsaMq6kQkbWyOwrRP62EvUfsWQ2H\nx1I6NmLpcrs43JOVD/Tab4q62VnXcc3ApSO6WMlTrlus77HgTDdyfCSAfCZjXxGG\nmOhjFjEKu7W14hThtv1Nsj+eIgVY6TI4IpTcbNV3Z8Z77j/2OJXPkc59qauMU/f/\nGJJAlf1ZAn9gMXTdiqaTDke4jjWYNkdM7KHzmv4VtniBCjAxvIEd5j9TEGXhUrw3\nDQIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Paras    */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAovpZy7M2FICHQb2y00oT\nsMsNVkVihzBOTteGJI8DsxiCN9vGksd7fZLYNpjI00yAkTQAaPhiY/6bgF01Tqos\n3TaZJwkQ/94t96ZmU7v3KEfZNbn4j9QH2T2oemutoNfK4fTNqbkHggAEOGOMpCXc\nNz616T8uJ56jhP7cN5wxjkg/p6hAc06eyGPHeeQh219UIA3sSOOYC8qEoq9pZEFa\nW4S5oVWn5uDQ8frlX3laBrSNpZnkPUfwvrqMimM8rznGhtefVYdigeXWS0HoZGjn\nT+Ul4s7zAj2nushwltrLIKNm67lGGxFy+KADwZ13Yrwpre/608Saqq5IsYtvBRgx\naQIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Indrones */ const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEApfAmqWxo1AU3XY96olP4\n10ibweRWo5u+joty+hk/+/1Ohr7dBA6/a1SL3BmjViju8PqtjSvpUdkLAvc0SJBT\nGU7oJyqXmS0T5qO0ArbnYjNHzlKZisenctxXjMchdIEgU84MqAwnvsokedLtm/xI\n0bWx8dUaG9BfRLtR7xOF28uSOQHpPSXcEUbPrHj6npvVdNoXY26yvmK0azv5MyBb\nZeYIT//c0niRBvHgEH1Ljvq5A/novysj2A4MnA5ReR5jMjsNi8cjEPxATJYnin9s\n9I2BPWkJNg2HKNzm6ISkbUTfeeaugYzGf7CmqY4wTFu5BPO4NPbuEN0hzNCuj4HD\n2wIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Thanos   */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA3lWV46SNjjHwhGKdMbIP\n5WktC/SSac1vjoeC+2A4gqCLwBw0pKTf2qIFka0BBLBJAigKRO5pyVtuJavQyrgh\n8ZcyhkdbJtDqKTqcVjlW7monZnh34W56iJva5rRXDg6oReSxKZheHGBxFOFYtiqW\nSF2sZHWqPGBBfD3MiAU5yymhdkZPHMmQX/tG76P6yf0tzUdsEEGOGgHA6M1Uxwad\nqc0Nu1wUimT6rPFksG0RofNDprn5Nwa1s8uCpadXqVG+g7mmGreIt0fGUSdZXy2T\n1qto7jcHome6UFfwmIuYzH4Awc0WdLzzh9KfiU0hzJMOQNkviAEfW9BQjbyG9NDW\n3wIDAQAB\n-----END PUBLIC KEY-----\n\0";
//*Drone Destination */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA3mgRUw5jy/h3xcyH/wXG\nOnxVt0j6JJc6sMn3RWox1s+gaXpDEcjo227Ld1M/iOf4QsA83XjbALkC/gCyHi26\n24xfNOximSnByuVTAVjL7RFJQJysbdcc+T/DY32VJfJhBaKRqQWSytIS67A6w4tS\nUC2XWdtUkD2jV+bMORbuc88vMdiDFsv3GjKad2RlB175lQcZKdoMbrD2JSM2Mn06\nF6SJtPLdzqxW7oCY36btW0xTzPyb8MRoKbFkeowPAtJPPbqtwv9DuTx6KNS8GMnV\nO7PF0lRftJi3O0H2oUdvyTpYRME6SOUrrBLKjnnZbSHiMpKmsdvyGimokrV72V8l\n4QIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Thanos latest*/const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxI7BYbJe8wGBRVYh0p2+\nQIJ2I40iiq08Qb5CkTpTIYbU944s0de89Dvw4qpEECfHBxay9U8baroYd9S+PnVl\nAUlUP3Y5NHjwlHmxG6dmFhBHN/rErgaeUZUMezJ5YjVd/qryUkc3WBQlO/FgRuXH\n1NV3QTonwE0/lAJVPcwpUTMlHVwZhIRuPypW9taugd6Dxbs6LLfMsWopnsgdoOHi\nK3aUPV2nL0DiOyFSephp3axPxBJI6YbyR0bNZsf/hLUAkkV8ZXaJ69YC4YWdaL2Y\nfxUAyeFmoLvVfhLXMdMu2f/eDbmMyNJS1pK1LjV4QW9EZsUpEreNsnZc0sfQj+rw\nsQIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Marut Drones*/const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsbWZ/4OxGAg7LiT5Qx7d\nqhpiqgV6g6jC2og32GvQgXT3ETpqbmeY5S49ke39v4LdM2SFszSOCwkZanjbu34/\nZMlf5BsyJQn+PVSqesLVJFI3j12J2JTQE82SHT4wQFLxoV3PdP/Z6AZEiJAk3joj\nmps+x9/DpodLfT9Hs7zfZ6Kzn2C1kn426w1DK+0yJJS3q/kCYfNodxfM8yHy3KgS\nZKwf29VEtvn5dMT7d4HrJXByg0phv905C0xecXoH0SxWgfYMsd8i755k3sX3STXD\nmr+63L4PcXpg3JuXWesoPQs3mIMG22INDZdCbu5z047iUHjTAQ/sX4j+4wEgXC9o\nSQIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*CUAV     */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxxhMmVCeICed2wWQ3JJW\n7jk0UOxfPpEBrM2DqgF9YyyXgKpckxkSoLz161KTLJU8qrKBa6XNRKqiAHPXA4B6\ne77eXGzcp7bAgOQFO45BrPg6UUUa6kQ3n9yVRRSITx2AboeN02Ue8PtS7hohRamb\nVwUTBL8bPZHqb7iV04R3ekswwWaOF1qmobNgf/Kx5ZVWeZZksA6GgTg+30OUWMuU\nquHYw0PJxref8FTbUhi1AtF3oqXoYx7WTEpa4nO1egbeFVudGtKA3myjV1nOF8/C\nLjqAOFZKBLtmtX5fKP9B/42WFnJQhM+n/LZ9SCtafjI0SQwtBVa+1jBok7p7LKDm\njQIDAQAB\n-----END PUBLIC KEY-----\n\0";
//*Aeronica*/const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzNd/Q3yqKZZr/lLNSp5v\nFJ0hp1gsaKMs1hTZ5a3QVcaS0Uw9ET04SjaBUCMkLcpVp/wPTYmwFC4ns73JS+Hh\n2dpkyrB3NWlga8p5+uLAqkgPznsB04DW3GCWu3hmgl18ZlF6v4Ul7NwSSKtgsH7Y\nYq1CIhqHtT0uPylpeEVgwebjHVluttf6I8yXgDexrsUQT23oIBq+zA4y+RX0CdyS\nYz/6YvCTbo0UmfiqmqRG7CMvzvyJHJEtpIXlgAwXGFVEZugdh7DIJQo2Xt9pFRWE\nEEAXSUX3SztpCjTCANixap67CUS0Zo3YKwowyliEwfPq9vrmMgGgOiJ1unBrsjrv\nrwIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Garuda aerospacee*/ const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA5/Nc3THHLNvpfc+MyrsW\nvAmaHUwffQpG5B6Gd+KW2rl50u3m98fwR9uJSpCvkH/yq4whH81YiSamPFdNG7dM\ndDKj6tCprMtIV5Aa2XSBwOFUJeXAT4GQ1ZTMH0Ye6FheMllfrJSjo8xc+/UrMW63\nFZaFnD+DJ2au1d/h2DlqxGqfAGJ6oeO2ocIXDdBI7hOskGR8kaUv75D/2qcd/csB\nYe6RQino2PQkesulgOeLuXtyAC8HoZiCGWDy+c6SNPYJrqeYPwzVE0hubNGOhY2o\nnE1Wm39TL038kwXyrmre+weQtJvctwRnhFNisPBZ3r0+fP9udkEIU1L7vVnwVqdM\nwQIDAQAB\n-----END PUBLIC KEY-----\n\0";
/*AeroBott*/ const char* pdrlPublicKey ="-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA5rqfXZKd7EiI9ofJNPRb\nHWdLJ6Gqjb0UK90HSuFNUuue6hJ/mDkbkdHinfEx6gCTBLX123OwFFPVH4T7FLWO\n3MeSmYGPFCfpdRXoTjtUo2TXmGKz3iiG9Upzc25nP7TeBTV0OKBJ6eok4vxWhAnq\npJostoZ2yJ/z61hPZ0Wh3kqp+Ybv9SuynF6XNgIm5n1O98zSBWi3bkl8/qterZIe\nBnewesenNd6g6CCgAG9lGqsacqv0ITPcx0kWysfUA/lwbzY8wYj4l+OpyrBz39uB\na7m2FcRCq9aJ1w8FXDPyjUDv/QnCt2DUWkkE40mJfQ9VPujwRoysa/M5pdtR9mM/\nnQIDAQAB\n-----END PUBLIC KEY-----\n\0";
//*UCAL   */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAp0KQPm39L6p1L8p6egfD\nlyr2qI7u7hS8b9GfFoLntTEHFGcNvqWu4Fl2CVAMu14H+VGDrjA6Fn6APZyxRUc0\nng+ibHvcDtjmQbP78CGGVzM/8yHfuEPAe4/pnmTGUtSlS79w4BdwliFLR7yr0ugl\nFNVgvpd/vGRnHwMorbyztksmTCmIeM4RW+1zj7orcj3peYRUxV9alb+2S8i8Yl65\nc1HUnv40wa14OOfpqSp8xOdoZ90Nr8lUxevA4NxyeTVsOm+N+dEWB2SW4BkUkFPR\nbBi4kHAwk19ILyGd/lO/RYdInbgFC/cKUiev6lfgGywhO5jBsq6nyclNB+k54A8t\nlQIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Airbots*/ const char* pdrlPublicKey ="-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAw/YfgeVcRGoBiGQKpWmh\njePlfQ783tv7S+33Si36xvhYtlm1idlSjTOM0OD5kvf6FsNxvM8cPV0UOaRwZyRH\nLLbwMeMDg54B/pqhNpgrRt+kdw1P0YYqHYSk8RJ7mdCC9EQYOOukVmGIUxsryup+\nMZzpR59m1ROwcAylZZP/GBgdQFCS4bFHvqYVHIJL0ccg5S6XHR1wNrmDbvF8PDZE\nBWeSzEDPEly9aU9LJ3Ogf/+KY7LJlvr4CvvyXpMGKlEViZS8JyFzbM18Q9mhAGzv\nfRc0tp+jhHj1wYvcTBQev3QlpwNkk1aJ7zt2U5airTOVqDgAVwL5sAO3X0kXJrNG\nuQIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*DTown*/ const char* pdrlPublicKey ="-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAi0xbdTE3adXzvM9A7jn7\ndgPUutF13n26P27Gtyw2WcCRMielLLB5meeuZkagtWT74BQEbWLN1T3aanSIuyHB\nAbzsMpmSZBe2G1fDt0pfGoVvov5ZYONuynt1FVKGu6c9kZRz7pjL5J7jTPyVTMLr\nRuW2e8srnWAz6ykpY37Q4ZGGQ6oydFRjCSoSENj59z8aNNnnEsyh/CRxC/BZOCSG\nYoeTBvKfTe6iZ2zY4gLmifZljYEAvgiTKdX+WhsUM588hDC1mNzP0EYqwGdw8fU4\nPWoTev5lxu+w30to8w+ZsnOlisnITcdixiwkOsPRBroYiTqmwr+eQI/12XenLnwU\n+QIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Dhaksha   */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAz7JOs5WPryovdCMScrYy\nPweAEHcOeegWlAkbc8+nr04F7F65LFqWIvKxaZfs879GClDWNyDCc/ilUN/fuLc4\nacnGS7lFc/Z1g2FQaaQQezUpPP9A+96+FK9CPnrOXL3oyWxXOQ/RtKeWEachw1ha\n2GIf/s3IZ4Yrf2HpbVXVz7WUZgi7N/sfBFdyLwYWLA6ODw8CMlGJ3BS0j8kr/lKV\n/m3onogbPWuyL5i/Z8sGkqZOzPWK4aq4V5efYl0OIqbnSAO6ANHK0un05kI7eJK8\n1qCZv+cUTfpRcg45U1eGUZVgMAM61x3+Zg24EX7WP45kMipYv7fEnHV1XP0UlKJb\nRQIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Tunga*/ const char* pdrlPublicKey ="-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAz7zWqSSQNcJqy91jx+5D\n9mKRc6tOu2W2CB3tcmocBVBxWomk3K+UxHiX0KI7P3nieFStg23XaJH5YUvX92GM\nLkzVBZcG3x03M6wNlWZN1d68u0nBaqe2P37WthizRn/asHMXgO6lz+ogZRNaFEhA\nD72VmEADVb7fnf3HB7cpNt0N9V3HSDxksQfLOsTNura8jxW9Q7FrYOaXhLUd/u1m\n1v3IHrJblnN/KbsDlNKStv3t6YQibkXZWb8t5pTf9r6C9DsiCJA/Ob12xFHiHhvi\nYMItEm3O+x6zsQX4LdXHRq+P0+nsJEpaewfGIcgwDMklTUh8PV4ww5lzO7lT1k5u\nTwIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Wow Go Green*/ const char* pdrlPublicKey ="-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAyy5Ei0OHSh1zl/kUt8nS\ndiPeSjIjv9X7LUwDze93UfHGgVGnod88H6+JvqbHe8w0qBKJ6pHTnHQjbK9rYvbv\nQD45f/lgukCSDspYE9np+OiWmmbI11t0QQ5OZGZA2xgkG15TISu8XHztbwF/MCk4\nwmImBJ8oKh7pmSJ1e0b6uubByvWX7hnB5vl0EFNe3ytR+udCStHzes9tjj98mlvd\nny3pA9SPDdpwiiNqTqJxxBKpgQYEiqcQzxHOfqpLTBVMpyXH61sTrM/oy942beUt\nq0wWHO/rCPobr89wvCLPmO20kGGzA8SYbE/WwUq2rvz+mFM6BM6DtoBVDVnlKrtc\nzQIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Erbatech*/ const char* pdrlPublicKey ="-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAurwJdEhlc/S5oLVyMxJF\nuJ9J0qz4UmUav/rkeoJuA8nyNe9EdwgM4GiGR+/R+l1h9APzcP1DvJ/NSxXSEl3d\nQ44o/fMA158+rTsasiKMMy/YwDdJrwdRr/hsBSDonoCRsiPyrcyNtSaVvHQwUGJr\n58cAVwaTCMGyEF2IJTWNt8G4y8UfZ4gaGpJQfy7bcVGHHM4+0AIwZnk7NFJZsBCQ\nIG9ic49GwE0Ioh2rYYKer2K7d40SgfwdB2LPWL9+VytBAnaLxJ3p2h2f1JkhehK3\nDSTsN27O2qdGRP+mETRYh7WXBgM8ilTVCOYBx/sKbc50sQmMh2NneZQD5Egrdt/x\nQQIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*HILD*/ const char* pdrlPublicKey ="-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA00XSYsHVM3zU5TD/0VF3\n0mtzCuW2xRAmxjXDII6DADypP8HETl307X3uPfhrE+9TlJfsChyW25WPMC3FBoBU\nXW1JXWQFNlhYQux1loUDjtFUgG4e4lLEBaZInWrT2n9GxdjzEaUdGOGjzY0B2QXm\nx2AkFPC7WUsvEj7XdeWh5533malc/HedMHh7Y3w3qPNfwn2HY2/313KTDONBSME5\nOci4FVU8M8qBB8mjp93h8F1E2I0I16grkfbRFtz07AJk1/mYsA3A0NdlXuVuZxql\nt+dG+1vX6vWXr6dwyZchYJXwV6Tkcjmd3fitJe+zgG1GVdwrsEE6zMnlIQXRFlO8\nyQIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Aerosys*/ const char* pdrlPublicKey ="-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqv4xfcynXhaa2BbMc8u3\nCNJccTPXibS3Opcfk9B3tpYJchCiwe/IviPfMJijDbRcbFXFAAniKHn+PrZE3mF/\n1PT6wpNu8ismeC3t9CmugUKSDdoHJUWFVvanIKnugHZc419Ghnp7VXx0Q1buTxIL\nL1ei5GDdPNcfpXPslb5SiZs7mQkZ8zTj6fydCkofdOITtly7oDMLrciWJoxhD9vJ\napdJxZPAKgXeMmxXUVqqHr5QlXH98lJEKv8Lqi8mva8EdRph2LEkJqrgWWJZl7a8\nP18kg3pPWFwewaHWRoTYEBiB/MwccP01W/L97JvPIFVTdva4Tr9H3Wcmt+kKH1JJ\n4QIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Multiplex*/ const char* pdrlPublicKey ="-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAy28CJ+7JHxAD6SkHS+Ms\nh8hKrOjHhPQHYiRm+J3ww3W5nImUvT1v92F2oNvONgFJNiJI86jJSmNixqKtEff5\n+tffs0ZnGQw8/f14MAIPGO6ewM5E4vRgTVYPlbhrWV9G5qmoomWLxHHt0tgqBBAM\naJz/Z0wfVZ9Lo1fH426fW5kfV4+nivkGPQLG2ZkexxiT1mtfSGW35mABnqncR42Q\nbnaWjS2R2jXr5tdP+2TnhuQfXHHVGhzpsEZ38RvFjIm12F+KDCoemwiCR1ocrrps\n/m/5hj7Y4cr40PCkZ/NIGARN6sPqqs7wo833wvohfsdi3CfYxDZSMD41KzlmMOFl\nDQIDAQAB\n-----END PUBLIC KEY-----\n\0";
///*Marucom*/ const char* pdrlPublicKey ="-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAwsMZ54PNjY6Jpoa0lS66\nicXzgIytx3JsTVpZC3rxsg08ZGYuoHI/9sDOD8hidnnVQ8hjOIoi8A3SdVXIxVbJ\nj4e1PUFh4PEZLtHPxB0tfdC4gZT7Uf3z5jbX2s8Vx6QTAuMhG/KqQ0Zc6/occH2S\nx7P5tdvBKiwqS/bM6BhJ7KiVe8YJ3kD5ngV20YDXL9jD7zzBCY6SeiU/4fUk9uAS\nkZnaGlGLODbU+9427Adt74eEzlcA/co2qZJ49wO0kRCq0z8a1YFqTeC9ZIJMhd+i\nXvJFfGqxBb0U48CUZbGcRLh68haK5Epo4WBn+7z/6JStzol6Vhxxw7ewdZXVzqms\ngwIDAQAB\n-----END PUBLIC KEY-----\n\0";
//* Vama Skylight */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzUVRf+Ix6ilQRwOeZw6p\npnHgj/1WKw0h6srWs4uFBikXhOinWESgA1eASyyMfltR3sFjKdQVkJKv1pdYlw9R\nQfFltjhHCOQg+y+kDwHeZGETLb93Sg2POzTesh87Qnr54+Z2Zndv4qbE4+U6SpZX\nQBjy9KunJJAjhRRjS40JTm5jwlSxwmz1t+esPk7WVujMKfm8wq723RIq0EewezkI\nEgk3lmLK6yfoslRC7aVPMv3rEpPZXWA5c0CnfQlowKCZdjH92oabco3B2bi6LWQ2\nvS1fo7Il9ZZdGH6VoXv2oDV5KHe3n5dI07822oz4oKBZ0JX4hzWhMGkDs21xajnA\nQQIDAQAB\n-----END PUBLIC KEY-----\n\0";
//* Indrone */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAgZiprbLFPlf/haCtsMHo\na+NYTrCvlONavZwtPMCbA1Uou8/QBLA5im3LvqHopouoKnpDMmBdpjSpIa4HgtR0\nkI0GsAgBtgVewUXHGf5mLw96LRudI/TPjay7kRoE0iLmOh0lHxM43duSWT4JuDIv\nDzWJi/cMm4RxNnqcrMiUQaZ3QurUMLPl4V2pnH+wY/f7ZK4UQ/o9cH/356aaINsD\nvYo4QoPrOhQmNMMmpdfxI2PGg0XHnyvMb8GKj0jLIZMfxBt2BqFPFxJlhnxtXs25\nmxa6iBTMBStoFFAwK0QFpbYkDsXwbYmp/un5RcbgqVTzisOQHyzsHte2tjsJJR+f\ngQIDAQAB\n-----END PUBLIC KEY-----\n\0";
//* Enercomp Solution */const char* pdrlPublicKey = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA0wiYPgp5xBxVCx170cFM\n0h+SfTwnc7WrTz1uydBpcN+KgTZn1Ts6wdhIRApasJdIU2EllvI4wfVJ4hkDZqur\n/qXtwu9O7vcLluRCR6N2ThBEJt82EgLcLDHdRfp8f5PodvuHNCtnY+VFlil/6vNW\nft5dCZINOfZq4SNs55PxmXBhokNZ5XbKM2dach7aBgoeOThsrIT7c+8qEcXE9JOm\n1J4SW/Bw14s6dj7FNnxqJVkaJYxf3u7RUdZ+TG/tBoOl4+xHT5W8Fdj5wgggtGDx\n63GpOEvBwSRVahZvBNt5n8PLt2yFhwRQ2WGMQP4pog9VO821WHKjZU5yhGBGJiSB\n8wIDAQAB\n-----END PUBLIC KEY-----\n\0";

//const AP_Param::GroupInfo AP_KEYSTORE::var_info[] = {
//		// @Param: TYPE
//		// @DisplayName: GPS type
//		// @Description: GPS type
//		// @Values: 16
//		// @RebootRequired: True
//		// @User: Advanced
//		AP_GROUPINFO("KEY_LEN",    0, AP_KEYSTORE, testParamKeystore, 16),
//
//		AP_GROUPEND
//};

AP_KEYSTORE* AP_KEYSTORE::getInstance()
{
	if (!m_pInstance)   // Only allow one instance of class to be generated.
		m_pInstance = new AP_KEYSTORE;
	return m_pInstance;
}

AP_KEYSTORE::AP_KEYSTORE()
{
	m_AP_Direct_Flash_Access = AP_Direct_Flash_Access::getInstance();
	mDevice = (keymaster_device_t)1;
	mState = KEYSTORE_IDEAL;
	strcpy((char*)salt,"salt");
	mRetry = 1;
	//	AP_Param::setup_object_defaults(this, var_info);

	//	unsigned char memory_buf[4000];
	//	mbedtls_memory_buffer_alloc_init( memory_buf, sizeof(memory_buf) );

	info_sha1 = mbedtls_md_info_from_type( MBEDTLS_MD_SHA1 );
	if( info_sha1 == NULL )
	{
		ret = 1;
	}

	if( ( ret = mbedtls_md_setup( &sha1_ctx, info_sha1, 1 ) ) != 0 )
	{
		ret = 1;
	}
	FLASH_Unlock();

	m_AP_Direct_Flash_Access->readFromInflash((uint8_t*)&isPasswordPresent,1,KEY_STORE_ISKEYPAIR_FLAGS);
	if( isPasswordPresent != 0x01 )
		isPasswordPresent = 0;
	else
	{
		m_AP_Direct_Flash_Access->readFromInflash((uint8_t*)&password,16,KEY_STORE_PASSWORD_ADD);
		isKeyPairPresent = true;
	}

	//if key genaration command received
	m_AP_Direct_Flash_Access->readFromInflash((uint8_t*)&isKeyPairGenarationCommand,1,KEY_STORE_ISCOMMAND_FOR_GENARATE_KEY);
	if( isKeyPairGenarationCommand != 0x01 )
		isKeyPairGenarationCommand = 0;
	else
	{
		eraseKeyStoreData();
		keyTransferTest = KEY_TRANSFER_GENARATE_KEY;
		isPasswordPresent = 0;
	}

	RNG_Config();
}

AP_KEYSTORE::~AP_KEYSTORE()
{
	mbedtls_md_free( &sha1_ctx );
}

void AP_KEYSTORE::setGenarateKeyFlagForReboot()
{
	eraseKeyStoreData();
	isKeyPairGenarationCommand = 1;
	isPasswordPresent = 0;
	isKeyPairPresent = 0;
	m_AP_Direct_Flash_Access->storeInflash((uint8_t*)&isKeyPairGenarationCommand,1,KEY_STORE_ISCOMMAND_FOR_GENARATE_KEY);
}

uint16_t AP_KEYSTORE::getKeyLen(KEY_TYPE keytype)
{
	uint16_t keylen = 0;
	switch(keytype)
	{
	case KEY_TYPE_PRIVATE:
		m_AP_Direct_Flash_Access->readFromInflash((uint8_t*)&keylen,2,KEY_STORE_PRIVATE_KEY_LEN_ADD);
		break;
	case KEY_TYPE_PUBLIC:
		m_AP_Direct_Flash_Access->readFromInflash((uint8_t*)&keylen,2,KEY_STORE_PUBLIC_KEY_LEN_ADD);
		break;
	case KEY_TYPE_EXTERNAL_PUBLIC:
		m_AP_Direct_Flash_Access->readFromInflash((uint8_t*)&keylen,2,KEY_STORE_EXTERNAL_PUBLIC_KEY_LEN_ADD);
		break;
	case KEY_PDRL_PUBLIC:
		break;
	}
	return keylen;
}

//return block count
uint16_t AP_KEYSTORE::bufferPadZero(unsigned char *keyBuf, uint16_t keylen)
{
	uint16_t pad = (16- (keylen % 16) );
	for(int i = 0; i < pad; i++)
	{
		keyBuf[keylen+i] = 0;
	}
	return (keylen+pad)/16;
}

bool AP_KEYSTORE::aesEncryptAndStoreBuffer(unsigned char *keyBuf, uint16_t keylen,uint32_t keyStoreAddress,uint32_t keyLenstoreAddress)
{
	if(isGrantAccess == true)
	{
		mbedtls_aes_context aes_ctx;
		//
		uint8_t key[AES_KEY_SIZE] = {0};

		getKeyStoreEncryptionPassword(key);

		mbedtls_aes_init( &aes_ctx );

		//store key length
		m_AP_Direct_Flash_Access->storeInflash((uint8_t*)&keylen,2,keyLenstoreAddress);

		uint16_t aesBlockCounts = bufferPadZero(keyBuf,keylen);
#if defined(STM32F767xx) || defined(STM32F427xx)
		for(int i = 0 ; i < aesBlockCounts; i++)
		{
			unsigned char plain[16];
			unsigned char cipher[32]={0};
			memcpy(plain,keyBuf+(16*i),16);
			mbedtls_aes_setkey_enc( &aes_ctx, key, 128);
			mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT,plain,cipher);
			m_AP_Direct_Flash_Access->storeInflash(cipher,16,keyStoreAddress+(16*i));
		}
#elif defined(STM32H743xx) || defined(STM32H757xx)
		for(int i = 0 ; i < aesBlockCounts; i++)
		{
			unsigned char plain[16];
			unsigned char cipher[32]={0};
			memcpy(plain,keyBuf+(16*i),16);
			mbedtls_aes_setkey_enc( &aes_ctx, key, 128);
			mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT,plain,cipher);

			i++;
			if( i < aesBlockCounts)
			{
				memcpy(plain,keyBuf+(16*i),16);
				mbedtls_aes_setkey_enc( &aes_ctx, key, 128);
				mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT,plain,cipher+16);
			}
			m_AP_Direct_Flash_Access->storeInflash(cipher,32,keyStoreAddress);
			keyStoreAddress = keyStoreAddress +32;
		}
#endif
		mbedtls_aes_free( &aes_ctx );
		return true;
	}
	return false;
}

bool AP_KEYSTORE::aesDecryptAndReadBuffer(unsigned char *keyBuf, uint16_t *keylen,uint32_t keyStoreAddress,uint32_t keyLenstoreAddress)
{
	if(isGrantAccess == true)
	{
		mbedtls_aes_context aes_ctx;

		uint8_t key[AES_KEY_SIZE] = {0};
		unsigned char dec_plain[16]={0};
		unsigned char cipher[16]={0};

		getKeyStoreEncryptionPassword(key);

		mbedtls_aes_init( &aes_ctx );

		//read key length
		m_AP_Direct_Flash_Access->readFromInflash((uint8_t*)keylen,2,keyLenstoreAddress);
		if( *keylen == 0xffff)
		{
			isPasswordPresent = 0;
			return false;
		}

		uint16_t aesBlockCounts = bufferPadZero(keyBuf,*keylen);

		for(int i = 0 ; i < aesBlockCounts; i++)
		{
			m_AP_Direct_Flash_Access->readFromInflash(cipher, 16, keyStoreAddress+(16*i));
			mbedtls_aes_setkey_dec(&aes_ctx, key, 128);
			mbedtls_aes_crypt_ecb( &aes_ctx, MBEDTLS_AES_DECRYPT, cipher, dec_plain );
			memcpy(keyBuf+(16*i),dec_plain,16);
		}
		mbedtls_aes_free( &aes_ctx );
		return true;
	}
	return false;
}

bool AP_KEYSTORE::storeKey(unsigned char *keyBuf, uint16_t keylen, KEY_TYPE keytype)
{
	if(isGrantAccess == true)
	{
		switch(keytype)
		{
		case KEY_TYPE_PUBLIC:
		{
			//store ley len in flash
			ret = aesEncryptAndStoreBuffer(keyBuf,keylen,KEY_STORE_PUBLIC_KEY_ADD,KEY_STORE_PUBLIC_KEY_LEN_ADD);
		}
		break;

		case KEY_TYPE_PRIVATE:
		{
			ret = aesEncryptAndStoreBuffer(keyBuf,keylen,KEY_STORE_PRIVATE_KEY_ADD,KEY_STORE_PRIVATE_KEY_LEN_ADD);
		}
		break;

		case KEY_TYPE_EXTERNAL_PUBLIC:
		{
			ret = aesEncryptAndStoreBuffer(keyBuf,keylen,KEY_STORE_EXTERNAL_PUBLIC_KEY,KEY_STORE_EXTERNAL_PUBLIC_KEY_LEN_ADD);
		}
		break;

		case KEY_PDRL_PUBLIC:
			break;
		}
		return ret;
	}
	return false;
}

bool AP_KEYSTORE::readKey(unsigned char *keyBuf, uint16_t* keylen, KEY_TYPE keytype)
{
	if((isGrantAccess == true) || (keytype==KEY_PDRL_PUBLIC))
	{
		switch(keytype)
		{
		case KEY_TYPE_PUBLIC:
		{
			//store ley len in flash
			aesDecryptAndReadBuffer(keyBuf,keylen,KEY_STORE_PUBLIC_KEY_ADD,KEY_STORE_PUBLIC_KEY_LEN_ADD);
		}
		break;

		case KEY_TYPE_PRIVATE:
		{
			aesDecryptAndReadBuffer(keyBuf,keylen,KEY_STORE_PRIVATE_KEY_ADD,KEY_STORE_PRIVATE_KEY_LEN_ADD);
		}
		break;

		case KEY_TYPE_EXTERNAL_PUBLIC:
		{
			aesDecryptAndReadBuffer(keyBuf,keylen,KEY_STORE_EXTERNAL_PUBLIC_KEY,KEY_STORE_EXTERNAL_PUBLIC_KEY_LEN_ADD);
		}
		break;
		case KEY_PDRL_PUBLIC:
		{
			keyBuf = (unsigned char *)pdrlPublicKey;
			*keylen = strlen(pdrlPublicKey);

		}
		break;
		}
		return true;
	}
	return false;
}

bool AP_KEYSTORE::getKeyStoreEncryptionPassword(uint8_t* pswd)
{
	// if possible discard result_key and read flash every time to get password
	memcpy(pswd,&result_key,AES_KEY_SIZE);
	return true;
}


bool AP_KEYSTORE::creatRSAKey()
{
#if ENABLERSA_KEY
	if(isKeyPairGenarated() == true)
		return true;
	ret = 1;
	size_t outBufSize = 0;
	const char *pers = "rsa_genkey";
	mbedtls_mpi N, P, Q, D, E, DP, DQ, QP;
	mbedtls_pk_context pk;

	mbedtls_ctr_drbg_init( &ctr_drbg );
	mbedtls_pk_init(&pk);
	pk.pk_ctx = &rsa;
	pk.pk_info = mbedtls_pk_info_from_type(MBEDTLS_PK_RSA);

	rsaPtr = mbedtls_pk_rsa( pk );
	mbedtls_rsa_init( rsaPtr, MBEDTLS_RSA_PKCS_V15, 0 );
	//	mbedtls_rsa_init( rsaPtr, MBEDTLS_RSA_PKCS_V21,0);

	mbedtls_mpi_init( &N ); mbedtls_mpi_init( &P ); mbedtls_mpi_init( &Q );
	mbedtls_mpi_init( &D ); mbedtls_mpi_init( &E ); mbedtls_mpi_init( &DP );
	mbedtls_mpi_init( &DQ ); mbedtls_mpi_init( &QP );

	mbedtls_entropy_init( &entropy );

	if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,(const unsigned char *) pers,
			strlen( pers ) ) ) != 0 )
	{
		//while(1);
		return false;
	}

		if( ( ret = mbedtls_rsa_gen_key( rsaPtr, mbedtls_ctr_drbg_random, &ctr_drbg, KEY_SIZE,
				EXPONENT ) ) != 0 )
		{
			//while(1);
			return false;
		}

		unsigned char output_buf[1700] = {0};
		if( ( ret = mbedtls_pk_write_pubkey_pem( &pk, output_buf, 2500, &outBufSize ) ) != 0 )
		{
			//while(1);
			return false;
		}

//		memcpy((char*)&output_buf,"-----BEGIN PUBLIC KEY-----\nMIIBIDANBgkqhkiG9w0BAQEFAAOCAQ0AMIIBCAKCAQEApW4Z0tBwfid903cOFXpQ\n33Zb3guVH/tdCXVkgX7/rmTxdbQuIDrrJFhQJ2NRApLM2SaU7DrfxGEfQtMcsF9g\nsgjHjKyt+AVSquDoOLcf5bRNSeTgiwy93fkT4EObIMAb6LDTY8knm5QTLzKeBjPw\nQvGOsljLZ9cwO/BTiSnXbvZHbTHX69hhoB6nnDlA6tacP4tfW//x9fyVmnCA2GAg\nvdNK06QBP+2ZX3YhX2x85Z8gbvOcTgZzFEFhGxkV/oEKszy3qiT2VsZbxiS8vTn6\nU7pN7NZFa3AC0KDfYHRe4DL+7QUI/6N2R9nwc/36WHlM90ibVO6B52Aa7c8OCwDF\nbQIBAw==\n-----END PUBLIC KEY-----\n",452);
//		outBufSize = 452;
		storeKey(output_buf,outBufSize,KEY_TYPE_PUBLIC);

		if( ( ret = mbedtls_pk_write_key_pem( &pk, output_buf, 2500, &outBufSize ) ) != 0 )
		{
			// while(1);
			return false;
		}
//		memcpy((char*)&output_buf,"-----BEGIN RSA PRIVATE KEY-----\nMIIEowIBAAKCAQEApW4Z0tBwfid903cOFXpQ33Zb3guVH/tdCXVkgX7/rmTxdbQu\nIDrrJFhQJ2NRApLM2SaU7DrfxGEfQtMcsF9gsgjHjKyt+AVSquDoOLcf5bRNSeTg\niwy93fkT4EObIMAb6LDTY8knm5QTLzKeBjPwQvGOsljLZ9cwO/BTiSnXbvZHbTHX\n69hhoB6nnDlA6tacP4tfW//x9fyVmnCA2GAgvdNK06QBP+2ZX3YhX2x85Z8gbvOc\nTgZzFEFhGxkV/oEKszy3qiT2VsZbxiS8vTn6U7pN7NZFa3AC0KDfYHRe4DL+7QUI\n/6N2R9nwc/36WHlM90ibVO6B52Aa7c8OCwDFbQIBAwKCAQAbklmjIr1qW+pN6S0D\nlGLP6Q9PrJjaqeTW6OYVlSqdENLo810FXycwuWKxOzgrGHd5hm4nXyVLZYU1zdod\nZTrIVsvsx3JUAOMcetFeyS/7ngzhpiVsgh+k/tilYJnayq9Rcs3l9tvvQ1iH3cUB\nCKgLKEJzDsyRTogJ/WNBhvk9KMdK7m9wBYocfiUq9q4KyVJLGC6shUNdfydAbviv\ngvNTx9JparLDrBytj8Quefw2zcihZR2PTzsq427h8faEdmgVLd4SaX7CgQRGubwn\ntq0g0jAAUX/LsbUjGE8aiut2/nwA/k5hWX6GgtlpMj+dgKYGITJOc6TkFmzqCgPr\nreQbAoGBAMmXNek5k/QNIsO+R8WNxfumexyO1Z3qERyF3nqU45S7iZQ0a3xjCv/S\nLgHYRy8xUCsTz43my83fFXM2rvjHwLBIiBuQVnqDCwi1UX+8mKIiR9QmbibjE+uW\n98tuFtL40xNCbrZELfd+6JtHaN3KA8ulgxC/wTBSe1HyLDGIWCxpAoGBANIUZVIS\nIzDoBwTcKWccWLLWfyrCCr/a8I2R+DImatgLmsgmuFRG2e176YZwSWRrfEGSxV0L\nX0Iz11SYvlYb+enroWerVYNEtTloeNQSWUlshVnEfx/CMdhnFzlTHAcdaQOkoHxz\nXIeh4Dy1Z57dcZ0s/ly+d3Q8Xjt/ZsT8lUBlAoGBAIZkzpt7t/gIwdfUL9kJLqfE\nUhMJ476cC2hZPvxjQmMnsQ14R6hCB1U2yVaQL3TLisdiil6Z3TPqDkzPH1CFKyAw\nWr0K5FGssgXONlUoZcFsL+LESW9CDUe5+oeeueH7N2IsSc7YHqT/RbzaRekxV90Z\nAgsqgMrhp4v2yCEFkB2bAoGBAIwNmOFhbMtFWgM9cO9oOyHkVMcsByqR9bO2pXbE\nRzqyZzAZ0DgvO/On8QRK25hHqCu3Lj4HlNbNOjhl1Dln+/FHwO/HjleDI3ua+zgM\nO4ZIWOaC/2qBdpBEuiY3aAS+RgJtwFL3kwUWlX3ORRSToRNzVD3UT6LS6Xz/mdio\nY4BDAoGBAKnz8MRZHGkhF/pti8lerEVBhWn+x22DWsL6j1Fqk4svyV4OJgXXeQpx\njKqqWG3lNAX2iWbdfQU2adaX7fINLmdnW1tELzik874AaQ7cp1IXgMFLism94XIK\nUwTdSfYwK5nwaN65WKFChd4TQ8kUAKW8okRI8K6mO7220TdzZwff\n-----END RSA PRIVATE KEY-----\n",1675);
//		outBufSize = 1676;
		storeKey(output_buf,outBufSize,KEY_TYPE_PRIVATE);
	isKeyPairPresent = true;

	mbedtls_mpi_free( &N ); mbedtls_mpi_free( &P ); mbedtls_mpi_free( &Q );
	mbedtls_mpi_free( &D ); mbedtls_mpi_free( &E ); mbedtls_mpi_free( &DP );
	mbedtls_mpi_free( &DQ ); mbedtls_mpi_free( &QP );
	//	rsa.P.p = 0;
	//	rsa.Q.p = 0;
	//	mbedtls_rsa_free( &rsa );
	mbedtls_ctr_drbg_free( &ctr_drbg );
	mbedtls_entropy_free( &entropy );
	return true;
#else
	return false;
#endif
}

//bool AP_KEYSTORE::creatRSAKey( )
//{
//	if(isKeyPairGenarated() == true)
//		return true;
//	ret = 1;
//	size_t outBufSize = 0;
//	const char *pers = "ec_genkey";
//	mbedtls_pk_context pk;
//	mbedtls_ecdsa_context ECDSA_Context;
//
//	mbedtls_pk_init(&pk);
//
//	mbedtls_pk_setup(&pk, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
//
//	mbedtls_entropy_init( &entropy );
//
//	mbedtls_ctr_drbg_init( &ctr_drbg );
//
//	if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,(const unsigned char *) pers,
//			strlen( pers ) ) ) != 0 )
//	{
//		//while(1);
//		return false;
//	}
//
//	mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_BP384R1,
//			mbedtls_pk_ec(pk),
//			mbedtls_ctr_drbg_random, &ctr_drbg
//	);
//
//	mbedtls_ecdsa_init( &ECDSA_Context );
//	mbedtls_ecdsa_from_keypair(&ECDSA_Context,mbedtls_pk_ec(pk));
//
//	unsigned char output_buf[2500];
//	if( ( ret = mbedtls_pk_write_pubkey_pem( &pk, output_buf, 2500, &outBufSize ) ) != 0 )
//	{
//		//while(1);
//		return false;
//	}
//	storeKey(output_buf,outBufSize,KEY_TYPE_PUBLIC);
//
//	if( ( ret = mbedtls_pk_write_key_pem( &pk, output_buf, 2500, &outBufSize ) ) != 0 )
//	{
//		// while(1);
//		return false;
//	}
//	storeKey(output_buf,outBufSize,KEY_TYPE_PRIVATE);
//
//	isKeyPairPresent = true;
//
//	mbedtls_ctr_drbg_free( &ctr_drbg );
//	mbedtls_entropy_free( &entropy );
//	return true;
//}

void AP_KEYSTORE::transferKey(int index,unsigned char *keyptr,uint16_t keyLen,uint16_t validDataLen)
{
	if(isGrantAccess == true)
	{
		memset(keyTransferBuf,0,sizeof(keyTransferBuf));
		memcpy(keyTransferBuf,keyptr+(KEY_TRANSFER_BLOCK_SIZE*index),KEY_TRANSFER_BLOCK_SIZE);
		keyTransferTXKeyLen = keyLen;
		keyTransferTXValidDataLen = validDataLen;
		keyTrasnFerTXBufferIndex = index;
		isKeyTransferTXBufferDirty = true;
		gcs().send_message(MSG_DATA_TRANSFER);
	}
}

void AP_KEYSTORE::receivKey(unsigned char *keyptr,uint16_t validDataLen,uint8_t bufferIndex)
{
	uint16_t numberOfBlockToSend = (( KEY_TRANSFER_BLOCK_SIZE-(validDataLen%KEY_TRANSFER_BLOCK_SIZE) )+validDataLen) / KEY_TRANSFER_BLOCK_SIZE;
	if( (numberOfBlockToSend > bufferIndex) && ( (keyTrasnFerRXBufferIndex+1) < bufferIndex ))
	{
		keyTrasnFerRXBufferIndex = 0;
		return;
	}
	keyTrasnFerRXBufferIndex = (uint16_t)bufferIndex;
	memcpy(extPublicKeyBuf+(bufferIndex*KEY_TRANSFER_BLOCK_SIZE),keyptr,KEY_TRANSFER_BLOCK_SIZE);

	if( (numberOfBlockToSend-1) == bufferIndex)
	{
		storeKey(extPublicKeyBuf,validDataLen,KEY_TYPE_EXTERNAL_PUBLIC);
		memset(extPublicKeyBuf,0,sizeof(extPublicKeyBuf));
		uint16_t keyl = 0;
		readKey(extPublicKeyBuf,&keyl,KEY_TYPE_EXTERNAL_PUBLIC);
	}
}

void AP_KEYSTORE::receivPAXML(char *paXmlPtr,uint16_t validDataLen,uint8_t bufferIndex)
{
#if ENABLE_PA
	volatile uint16_t numberOfBlockToSend = (( KEY_TRANSFER_BLOCK_SIZE-(validDataLen%KEY_TRANSFER_BLOCK_SIZE) )+validDataLen) / KEY_TRANSFER_BLOCK_SIZE;
	//	if( (numberOfBlockToSend > bufferIndex) && ( (keyTrasnFerRXBufferIndex+1) < bufferIndex ))
	//	{
	//		keyTrasnFerRXBufferIndex = 0;
	//		return;
	//	}
	keyTrasnFerRXBufferIndex = bufferIndex;
	if(keyTrasnFerRXBufferIndex == 0)
	{
		memset(paXmlBuffer,0,sizeof(paXmlBuffer));
	}
	memcpy(paXmlBuffer+(keyTrasnFerRXBufferIndex*KEY_TRANSFER_BLOCK_SIZE),paXmlPtr,KEY_TRANSFER_BLOCK_SIZE);

	if((numberOfBlockToSend-1) == keyTrasnFerRXBufferIndex)
	{
		//		uint16_t keyl = 0;
		//		readKey(extPublicKeyBuf,&keyl,KEY_TYPE_EXTERNAL_PUBLIC);
		AP_LIBNPNT::getInstance()->authenticateSinglePA(paXmlBuffer,validDataLen);
		AP_PDRL_Logger::getInstance()->start_new_geofence_log(AP_LIBNPNT::getInstance()->npnt_handle.params.permissionArtifactID);
	}
	ackCommand = MAVLINK_MSG_ID_DATA_TRANSFER;
	ackCommandType = DATA_TYPE_PA;
	ack = 1;
	gcs().send_message(MSG_ACK_FOR_COMMAND);
	//	mavlink_msg_ack_for_command_send(
	//			(mavlink_channel_t)0,
	//			keyStore->ackCommand,
	//			keyStore->ackCommandType,
	//			keyStore->ack
	//			);
#endif
}

void AP_KEYSTORE::openKeystore()
{

}

bool AP_KEYSTORE::generateSalt(char *saltV,uint16_t * len)
{
	memcpy(saltV,salt,SALT_SIZE);
	*len = SALT_SIZE;
	return true;
}

void AP_KEYSTORE::closeKeystore()
{
	isGrantAccess = 0;
}

void AP_KEYSTORE::getPlainPassword(uint8_t* pswd)
{
	memcpy(pswd,password,16);
}

void AP_KEYSTORE::genaratePasswordPhrase(uint8_t* pswd)
{
	//genarate randome number of 16 byte
	for(int i = 0; i < 16; i++)
	{
		pswd[i] = (uint8_t)( RNG_GetRandomNumber() % 100 + 1);
	}
	m_AP_Direct_Flash_Access->storeInflash(pswd,16,KEY_STORE_PASSWORD_ADD);
}

void AP_KEYSTORE::clearePassword()
{
	isPasswordPresent = 0;
}

bool AP_KEYSTORE::isPasswordGenarated()
{
	return isPasswordPresent;
}

bool AP_KEYSTORE::isKeyPairGenarated()
{
	return isKeyPairPresent;
}

bool AP_KEYSTORE::getGrant(uint8_t* pswd, uint16_t pswdlen)
{
	uint8_t key[16] = { 1 ,2 ,3,4 ,5 ,6 ,7, 8, 9,10,11,12,13,14,15,16};

	ret = mbedtls_pkcs5_pbkdf2_hmac( &sha1_ctx, pswd, pswdlen, salt,slen, it_cnt, key_len, key );

	if( ret != 0 || memcmp( result_key, key, key_len ) != 0 )
	{
		ret = 1;
		goto exit;
	}
	isGrantAccess = true;
	return true;
	exit:
	return false;
}

#include "flash.h"
void AP_KEYSTORE::eraseKeyStoreData()
{

#ifdef STM32H743xx
	//FLASH_Erase_Sector(14,FLASH_BANK_2, FLASH_VOLTAGE_RANGE);
	//stm32_flash_erasepage(23);
	stm32_flash_erasepage(14);
#endif

#ifdef STM32F767xx
	FLASH_Unlock();
	stm32_flash_erasepage(11);
	FLASH_Lock();
#endif

#ifdef STM32F427xx
	FLASH_Unlock();
	if (FLASH_EraseSector(KEY_STORE_STARTING_SECTOR, FLASH_VOLTAGE_RANGE) != FLASH_COMPLETE)
	{
	}
	FLASH_Lock();
#endif
}

void AP_KEYSTORE::generateKeyFromPassword(uint8_t* pswd)
{
	if(isPasswordPresent == 0)
	{
		eraseKeyStoreData();
		genaratePasswordPhrase(password);
		ret = mbedtls_pkcs5_pbkdf2_hmac( &sha1_ctx, password, plen, salt,slen, it_cnt, key_len, result_key );
		if( ret != 0 )
		{
			ret = 1;
			return;
		}

		//store hashed keystore password in flash
		m_AP_Direct_Flash_Access->storeInflash(result_key,16,KEY_STORE_HASH_PASSWORD_ADD);

		isPasswordPresent = 1;
		m_AP_Direct_Flash_Access->storeInflash((uint8_t*)&isPasswordPresent,1,KEY_STORE_ISKEYPAIR_FLAGS);
	}
	else
	{
		m_AP_Direct_Flash_Access->readFromInflash(result_key,16,KEY_STORE_HASH_PASSWORD_ADD);
		m_AP_Direct_Flash_Access->readFromInflash(password,16,KEY_STORE_PASSWORD_ADD);
	}
	memcpy(pswd,password,plen);
	//	getGrant(password,plen);
}

void AP_KEYSTORE::setAllowedFlashFirmware()
{
	isAllowedToFlashFirmware = 1;
	m_AP_Direct_Flash_Access->storeInflash((uint8_t*)&isAllowedToFlashFirmware,1,IS_ALLOWED_TO_FLASH_FIRMWARE);
}

uint8_t AP_KEYSTORE::getAllowedFlashFirmware()
{
	m_AP_Direct_Flash_Access->readFromInflash((uint8_t*)&isAllowedToFlashFirmware,1,IS_ALLOWED_TO_FLASH_FIRMWARE);
	return isAllowedToFlashFirmware;
}

bool AP_KEYSTORE::verifyFirmwareSignature(uint8_t* firmwareSignature, uint8_t* firmwareHash)
{

#if 0
	//unsigned char* pdrlPublicKeyPtr = 0;
	uint16_t keyLen = strlen(pdrlPublicKey)+1; //added extra null termination count
	//keyStore->readKey(&pdrlPublicKeyPtr,&keyLen,KEY_PDRL_PUBLIC);
	mbedtls_pk_context pk;
	mbedtls_pk_init(&pk);
	volatile int err = 0;
	//ret = mbedtls_pk_parse_key(&pk,(unsigned char *)pdrlPublicKey,keyLen,NULL,0);
	err = mbedtls_pk_parse_public_key(&pk,(unsigned char *)pdrlPublicKey,keyLen);
	if(err != 0 )
	{
		mbedtls_pk_free(&pk);
		return false;
	}
	err = mbedtls_pk_verify(&pk,MBEDTLS_MD_SHA256,(unsigned char *)firmwareHash,32,(unsigned char *)firmwareSignature,256);
	if( err != 0 )
	{
		mbedtls_pk_free(&pk);
		return false;
	}
	mbedtls_pk_free(&pk);
	return true;
#endif

	int err = 0;
	mbedtls_pk_context pk;

	mbedtls_pk_init( &pk );

	if( ( err = mbedtls_pk_parse_public_key( &pk,(const unsigned char*)pdrlPublicKey,strlen((char*)pdrlPublicKey)+1) ) != 0 )
	{
		gcs().send_text(MAV_SEVERITY_ERROR, "%s", "Firmware signature: verify failed");
		mbedtls_pk_free(&pk);
		return false;
	}

	if( ( err = mbedtls_pk_verify( &pk, MBEDTLS_MD_SHA256, firmwareHash,32, firmwareSignature, 256 ) ) != 0 )
	{
		gcs().send_text(MAV_SEVERITY_ERROR, "%s", "Firmware signature: verify failed");
		mbedtls_pk_free(&pk);
		return false;
	}

	gcs().send_text(MAV_SEVERITY_ERROR, "%s", "Firmware signature: verify success");
	mbedtls_pk_free(&pk);
	setAllowedFlashFirmware();
	return true	;

	//    int ret = 1;
	//    unsigned c;
	//    int exit_code = MBEDTLS_EXIT_FAILURE;
	//    size_t i;
	//    mbedtls_rsa_context rsa;
	//    unsigned char hash[32];
	//    unsigned char buf[MBEDTLS_MPI_MAX_SIZE];
	//    char filename[512];
	//
	//    mbedtls_rsa_init( &rsa, MBEDTLS_RSA_PKCS_V15, 0 );
	//
	//    if( ( ret = mbedtls_mpi_read_file( &rsa.N, 16, f ) ) != 0 ||( ret = mbedtls_mpi_read_file( &rsa.E, 16, f ) ) != 0 )
	//    {
	//        goto exit;
	//    }
	//
	//    rsa.len = ( mbedtls_mpi_bitlen( &rsa.N ) + 7 ) >> 3;
	//
	//    fclose( f );
	//
	//    /*
	//     * Extract the RSA signature from the text file
	//     */
	//    mbedtls_snprintf( filename, sizeof(filename), "%s.sig", argv[1] );
	//
	//    if( ( f = fopen( filename, "rb" ) ) == NULL )
	//    {
	//        mbedtls_printf( "\n  ! Could not open %s\n\n", filename );
	//        goto exit;
	//    }
	//
	//    i = 0;
	//    while( fscanf( f, "%02X", (unsigned int*) &c ) > 0 &&
	//           i < (int) sizeof( buf ) )
	//        buf[i++] = (unsigned char) c;
	//
	//    fclose( f );
	//
	//    if( i != rsa.len )
	//    {
	//        mbedtls_printf( "\n  ! Invalid RSA signature format\n\n" );
	//        goto exit;
	//    }
	//
	//    /*
	//     * Compute the SHA-256 hash of the input file and
	//     * verify the signature
	//     */
	//    mbedtls_printf( "\n  . Verifying the RSA/SHA-256 signature" );
	//    fflush( stdout );
	//
	//    if( ( ret = mbedtls_md_file(
	//                    mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ),
	//                    argv[1], hash ) ) != 0 )
	//    {
	//        mbedtls_printf( " failed\n  ! Could not open or read %s\n\n", argv[1] );
	//        goto exit;
	//    }
	//
	//    if( ( ret = mbedtls_rsa_pkcs1_verify( &rsa, NULL, NULL, MBEDTLS_RSA_PUBLIC,
	//                                  MBEDTLS_MD_SHA256, 20, hash, buf ) ) != 0 )
	//    {
	//        mbedtls_printf( " failed\n  ! mbedtls_rsa_pkcs1_verify returned -0x%0x\n\n", (unsigned int) -ret );
	//        goto exit;
	//    }
	//
	//    mbedtls_printf( "\n  . OK (the signature is valid)\n\n" );
	//
	//    exit_code = MBEDTLS_EXIT_SUCCESS;
	//
	//
	//exit:
	//    mbedtls_rsa_free( &rsa );

}

namespace AP {

AP_KEYSTORE &key_store()
{
	return *AP_KEYSTORE::getInstance();
}

}
