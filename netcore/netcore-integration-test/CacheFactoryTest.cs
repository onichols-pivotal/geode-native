/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
using System;
using Xunit;
using Xunit.Abstractions;

namespace Apache.Geode.Client.IntegrationTests
{
  [Collection("Geode .Net Core Collection")]
  public class CacheFactoryTest : TestBase {
    public CacheFactoryTest(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
    {
    }

    [Fact]
    public void CreateFactoryNotNull() {
      using (var cacheFactory = CacheFactory.Create()) {
        Assert.NotNull(cacheFactory);
      }
    }

    [Fact]
    public void CacheFactoryGetVersion() {
      using (var cacheFactory = CacheFactory.Create()) {
        var version = cacheFactory.Version;
        Assert.NotEqual(version, String.Empty);
      }
    }

    [Fact]
    public void CacheFactoryGetProductDescription() {
      using (var cacheFactory = CacheFactory.Create()) {
        var description = cacheFactory.ProductDescription;
        Assert.NotEqual(description, String.Empty);
      }
    }

    [Fact]
    public void CacheFactorySetPdxIgnoreUnreadFields() {
      using (var cacheFactory = CacheFactory.Create()) {
        cacheFactory.PdxIgnoreUnreadFields = true;
        cacheFactory.PdxIgnoreUnreadFields = false;
      }
    }

    [Fact]
    public void CacheFactorySetPdxReadSerialized() {
      using (var cacheFactory = CacheFactory.Create()) {
        cacheFactory.PdxReadSerialized = true;
        cacheFactory.PdxReadSerialized = false;
      }
    }

    [Fact]
    public void CacheFactoryCreateCacheNotNull() {
      using var cacheFactory = CacheFactory.Create();
      Assert.NotNull(cacheFactory);
      using var cache = cacheFactory.CreateCache();  // lgtm[cs / useless - assignment - to - local]
      Assert.NotNull(cache);
    }

    [Fact]
    public void CacheFactorySetProperty() {
      using var cacheFactory = CacheFactory.Create();
      Assert.NotNull(cacheFactory);
      cacheFactory.SetProperty("log-level", "none").SetProperty("log-file", "geode_native.log");
    }
  }
}
