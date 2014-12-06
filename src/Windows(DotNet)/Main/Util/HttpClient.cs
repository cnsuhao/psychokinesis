/* ==============================================================================
 * 作者：张扬
 * 创建时间：2014/9/7 23:17:04
 * 简介：HTTP客户端
 * ==============================================================================*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Http;
using System.Net.Http.Headers;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace Psychokinesis.Main.Util
{
    class HttpClient
    {
        public static string Get(string url)
        {
            System.Net.Http.HttpClient client = new System.Net.Http.HttpClient();
            var response = client.GetAsync(url);
            response.Result.EnsureSuccessStatusCode();

            return response.Result.Content.ReadAsStringAsync().Result;
        }

        public static string Post(string url, IEnumerable<KeyValuePair<string, string>> form)
        {
            System.Net.Http.HttpClient client = new System.Net.Http.HttpClient();
            var postData = new System.Net.Http.FormUrlEncodedContent(form);

            var response = client.PostAsync(url, postData);
            response.Result.EnsureSuccessStatusCode();

            return response.Result.Content.ReadAsStringAsync().Result;
        }

        public static string PostJson(string url, JObject json)
        {
            System.Net.Http.HttpClient client = new System.Net.Http.HttpClient();
            client.DefaultRequestHeaders.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));
            string postBody = JsonConvert.SerializeObject(json);

            var response = client.PostAsync(url, new StringContent(postBody, Encoding.UTF8, "application/json"));
            response.Result.EnsureSuccessStatusCode();

            return response.Result.Content.ReadAsStringAsync().Result;
        }
    }
}
